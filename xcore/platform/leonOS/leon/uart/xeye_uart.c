/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

/**********************************************************************
* Two kinds of uart APIs, one for baremetal, one for OS. Baremetal APIs
* are mainly for printf under uart purpose, OS APIs are for data send and
* receive purpose. When printf to uart, the xeye_drv_uart_init() must be
* called before any printf() function.
* [IMPORTANT]: When using OS APIs, app must initialise the uart clock first,
*              And define macro DISABLE_PRINTF_TO_UART in makefile at the
*              same time.
*
* baremetal:xeye_drv_uart_gpio_cfg()
*           xeye_drv_uart_init()
*           xeye_drv_uart_echo()
*       OS: xeye_os_uart_init()
*           xeye_os_uart_open()
*           xeye_os_uart_close()
*           xeye_os_uart_read()
*           xeye_os_uart_write()
*           xeye_os_uart_config_sw_fifo()
*           xeye_os_uart_config_baudrate()
*           xeye_os_uart_config_loopback()
************************************************************************/
#include "xeye_uart.h"
#define MVLOG_UNIT_NAME xeye_uart
#include <mvLog.h>

#define INTERRUPTLEVEL          (7)
#define BAUDRATE                (115200)
// [NOTICE]: The MAX_RXTXFIFO should not be too small
#define MAX_RXTXFIFO            (2560)

static uint8_t gs_uart_tx_fifo[MAX_RXTXFIFO] __attribute__((aligned(64)));
static uint8_t gs_uart_rx_fifo[MAX_RXTXFIFO] __attribute__((aligned(64)));

static tyAuxClkDividerCfg gs_aux_clk_cfg_uart[] = {
    {
        .auxClockEnableMask     = AUX_CLK_MASK_UART,
        .auxClockSource         = CLK_SRC_PLL0,
        .auxClockDivNumerator   = 1,
        .auxClockDivDenominator = 1,
    } ,
    { 0, (tyHglCprClockSrc)0, 0, 0 }, // Null Terminated List
};

static tDrvUartCfg gs_uart_cfg = {
    .DataLength = UartDataBits8,
    .StopBits   = UartStopBit1,
    .Parity     = UartParityNone,
    .BaudRate   = UART_BAURD_RATE,
    .LoopBack   = No,
};

void xeye_drv_uart_gpio_cfg() {
    DrvGpioSetMode(33, D_GPIO_DIR_IN  | D_GPIO_MODE_4);  // Configure GPIO: 33:sin
    DrvGpioSetMode(31, D_GPIO_DIR_OUT | D_GPIO_MODE_4);  // Configure GPIO: 31:sout
    return;
}

int xeye_drv_uart_init(u32 baudrate) {

    int desiredClk = baudrate * 16;
    int ret = 0;
    s32 inputClkKhz = -1;
    tyClockType inClkType;
    tyCprClockSrc inClkSrc;

    xeye_drv_uart_gpio_cfg();

    //change clock
    inClkType = PLL0_OUT_CLK;
    inClkSrc = CLK_SRC_PLL0;
    inputClkKhz = DrvCprGetClockFreqKhz(inClkType, NULL);

    if (inputClkKhz == -1) {
        mvLog(MVLOG_ERROR, "Error getting input clock");
        return -1;
    }

    int64_t inputClk = inputClkKhz * 1000;

    unsigned smallestError = -1u, bestNum = 1, bestDen = 1;

    // num, den can be max 4095 (12 bits)
    // constraint: (num == den) or (2*num <= den)
    if (desiredClk != inputClk) {
        for (int num = 1; num < 2048; num++) {
            for (int den = 2 * num; den < 4096; den++) {
                unsigned error = abs(inputClk * num / den - desiredClk);

                if (error < smallestError) {
                    smallestError = error;
                    bestNum = num;
                    bestDen = den;
                }

                if (error == 0) {
                    break;    // Exact match
                }
            }
        }
    }

    gs_aux_clk_cfg_uart[0].auxClockDivNumerator = bestNum;
    gs_aux_clk_cfg_uart[0].auxClockDivDenominator = bestDen;

    DrvCprAuxClockArrayConfig(gs_aux_clk_cfg_uart);

    gs_uart_cfg.BaudRate   = baudrate;

    // Init the UART
    ret = DrvUartInit(&gs_uart_cfg, desiredClk);

    return ret;
}

// Echo back received character
void xeye_drv_uart_echo() {
    u8 d;
    int size;

    while (1) {
        DrvUartRxData(&d, &size, 1);

        if (size != 0) {
            DrvUartTxData(&d, 1);
        }
    }
}

int xeye_os_uart_init(u32 baudrate) {
    int ret = 0;
    rtems_device_major_number uartMajor;
    struct osUartInitSettings_t uartInit;

    uartInit.interruptLevel = INTERRUPTLEVEL;
    uartInit.uartCfg.BaudRate = baudrate;
    uartInit.uartCfg.DataLength = UartDataBits8;
    uartInit.uartCfg.Parity = UartParityNone;
    uartInit.uartCfg.StopBits = UartStopBit1;
    // TODO(huyuexiang): MUST enable loopback when initialising to avoid data received
    //                   before do read implementation.
    // [NOTICE]: DON'T forget disable loopback later!
    uartInit.uartCfg.LoopBack = Yes;

    xeye_drv_uart_gpio_cfg();
    //register driver functions and get device major ID, device name:/dev/uart
    ret = rtems_io_register_driver(0, &uartDrvTbl, &uartMajor);

    if (ret != RTEMS_SUCCESSFUL) {
        mvLog(MVLOG_ERROR, "ERROR uart init rtems_io_register_driver failed:%d", ret);
        return ret;
    }

    //initialise hardware
    ret = rtems_io_initialize(uartMajor, 0, &uartInit);

    if (ret != RTEMS_SUCCESSFUL) {
        mvLog(MVLOG_ERROR, "ERROR uart init rtems_io_initialize failed: %d", ret);
        return ret;
    }

    return ret;
}

int xeye_os_uart_open(void) {
    int fd = 0;
    int i;
    fd = open("/dev/uart", O_RDWR,
              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd <= 0) {
        mvLog(MVLOG_ERROR, "ERROR opening uart!");
    }

    // [NOTICE]: Do nothing, this line is used to fix receive failure bug.
    for (i = 0; i < 100; i++);

    xeye_os_uart_config_sw_fifo(fd);
    xeye_os_uart_config_loopback(fd, UART_LOOPBACK_DISABLED);
    return fd;
}

int xeye_os_uart_close(int fd) {
    int ret = 0;

    if (fd <= 0) {
        mvLog(MVLOG_ERROR, "ERROR uart close invalid fd!");
        return -1;
    }

    ret  = close(fd);

    if (ret) {
        mvLog(MVLOG_ERROR, "ERROR closing uart!");
    }

    return ret;
}

int xeye_os_uart_read(int fd, uint8_t* const buf, int max_len) {
    int real_len = 0;

    if (fd <= 0 || buf == NULL ||
            max_len < 0 || max_len > MAX_RXTXFIFO) {
        mvLog(MVLOG_ERROR, "ERROR uart read invalid params!");
        return -1;
    }

    real_len = read(fd, buf, max_len);
    return real_len;
}

int xeye_os_uart_write(int fd, uint8_t const* buf, int len) {
    int real_len = 0;

    if (fd <= 0 || buf == NULL ||
            len < 0 || len > MAX_RXTXFIFO) {
        mvLog(MVLOG_ERROR, "ERROR uart write invalid params!");
        return -1;
    }

    real_len = write(fd, buf, len);
    return real_len;
}

int xeye_os_uart_config_sw_fifo(int fd) {
    int ret = 0;
    int ctl;
    struct osUartInitSettings_t uartInit;

    uartInit.fifo.rxFifoLocation = gs_uart_rx_fifo;
    uartInit.fifo.rxFifoSize = MAX_RXTXFIFO;
    uartInit.fifo.txFifoLocation = gs_uart_tx_fifo;
    uartInit.fifo.txFifoSize = MAX_RXTXFIFO;

    ctl = UART_SW_FIFO_ENABLED;
    ret = ioctl(fd, ctl, &uartInit);

    return ret;
}
int xeye_os_uart_config_baudrate(s32 fd, u32 baudrate) {
    int32_t ret;
    uint32_t ctl;

    ctl = UART_SET_BAUDRATE;
    ret = ioctl(fd, ctl, baudrate);

    if (ret < 0) {
        mvLog(MVLOG_ERROR, "ERROR set baudrate uart!");
    }

    return ret;
}
int xeye_os_uart_config_loopback(s32 fd, uartIoctls_t ctl) {
    if (ctl != UART_LOOPBACK_ENABLED && ctl != UART_LOOPBACK_DISABLED) {
        mvLog(MVLOG_ERROR, "ERROR invalid loopback command!");
        return -1;
    }

    return ioctl(fd, ctl);
}