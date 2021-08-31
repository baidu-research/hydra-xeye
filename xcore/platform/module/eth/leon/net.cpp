/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <rtems.h>
#include <rtems/bspIo.h>
#include <fcntl.h>
#include "rtems/rtems_bsdnet.h"
#include <assert.h>
#include "sys/types.h"
#include <sys/socketvar.h>
#include "sys/socket.h"
#include <netinet/in.h>
#include "netdb.h" //gethostbyname
#include <sys/ioctl.h>
#include "net/if.h"
#include "net/if_var.h"
#include "arpa/inet.h"
#include "sys/proc.h"
#include <bsp.h>
#include "DrvGpio.h"
#include "DrvI2cMaster.h"
#include "DrvI2c.h"
#include "OsDrvCpr.h"
#include "OsDrvTimer.h"
#include "rtems/dhcp.h"
#include "time.h"
#include "mvLog.h"
#include "net.h"
#include <string>
#define RTEMS_BSP_NETWORK_DRIVER_NAME      "gr_eth1"
#define RTEMS_BSP_NETWORK_DRIVER_ATTACH    rtems_leon_greth_gbit_driver_attach

#define RDA_COUNT    96 //number of Rx descriptors
#define TDA_COUNT    96 //number of Tx descriptors

// The interrupt priority that the ETH will have, 1 (low) - 15 (high)
#define ETH_ISR_PRIORITY    5

#define IO_RXD_0     (69)
#define IO_RXD_1     (70)
#define IO_RXD_2     (71)
#define IO_RXD_3     (72)
#define IO_RXD_4     (73)
#define IO_RXD_5     (46)
#define IO_RXD_6     (47)
#define IO_RXD_7     (48)

#define IO_TXCLK     ( 1)
#define IO_TX_EN     ( 2)
#define IO_TXER      ( 3)
#define IO_RXCLK     ( 4)
#define IO_RXDV      ( 5)
#define IO_RXER      ( 6)
#define IO_RXCOL     ( 7)
#define IO_RXCRS     ( 8)
#define IO_GTXCK     (35)
#define IO_125CLK    ( 0)

#define IO_TXD_0     (11)
#define IO_TXD_1     (62)
#define IO_TXD_2     (63)
#define IO_TXD_3     (64)
#define IO_TXD_4     (65)
#define IO_TXD_5     (66)
#define IO_TXD_6     (67)
#define IO_TXD_7     (68)

#define IO_MDC       ( 9)
#define IO_MDIO      (10)

// I2C
#define I2C3_SDA     (80)
#define I2C3_SCL     (79)
#define I2C3_SPEED_KHZ_DEFAULT  (400)
#define I2C3_ADDR_SIZE_DEFAULT  (ADDR_7BIT)
#define WM8325_I2C              ((0x6C)>>1)

// This have a big impact in Gigabit mode (at least for loopback test), set to  1 or 0
#define INVERT_GTX_CLK_CFG  1

// Memory allocated for mbufs
#define APP_MBUF_ALLOCATION       (2*64*1024)
// Memory allocated for mbuf clusters
#define APP_MBUFCLUSTERALLOCATION (3*128*1024)
// MAC ADDRESS
#define APP_MACADDRESS            "\x94\xDE\x80\x6B\x12\x11"

#define SET_BIT(reg, bit, val)    (val?(reg |= 1<<bit):(reg &= ~(1<<bit)))

#define HTTP_GET_REQUEST          "GET / HTTP/1.0\r\n\r\n"

#define CLIENTHOSTNAME            "M2"

#define IP_ADDR     "192.168.1.219"
#define NET_MASK    "255.255.0.0"

#ifdef __cplusplus
extern "C" {
#endif
#include <bsp/greth_gbit.h>
extern int rtems_leon_greth_gbit_driver_setup(rtems_greth_gbit_hw_params* params);
#ifdef __cplusplus
}
#endif

extern char lrt_g_ucMac[6];
struct rtems_bsdnet_ifconfig ifconfig =
{
    RTEMS_BSP_NETWORK_DRIVER_NAME, 	//name
    RTEMS_BSP_NETWORK_DRIVER_ATTACH,  // attach function
    0, // link to next interface
    NULL, //IP address is resolved by DHCP server
    NULL, //IP net mask is resolved by DHCP server
    lrt_g_ucMac,
    0, //broadcast
    0, //mtu
    0, //rbuf_count
    0, //xbuf_count
    0, //port
    0, //irno
    0, //bpar
    NULL //Driver control block pointer
};

struct rtems_bsdnet_config rtems_bsdnet_config =
{
    &ifconfig,
    rtems_bsdnet_do_dhcp,
    0, // Default network task priority
    APP_MBUF_ALLOCATION,//Default mbuf capacity
    APP_MBUFCLUSTERALLOCATION,//Default mbuf cluster capacity
    CLIENTHOSTNAME,//Host name
    NULL,//Domain name
    NULL, //Gateway is resolved by DHCP server
    NULL,//Log host
    {NULL},//Name server(s)
    {NULL},//NTP server(s)
    0, //sb_efficiency
    0, //udp_tx_buf_size
    0, //udp_rx_buf_size
    0, //tcp_tx_buf_size
    0, //tcp_rx_buf_size
};

static u32 commonI2CErrorHandler(I2CM_StatusType i2cCommsError, u32 slaveAddr, u32 regAddr);

// i2C data
static tyI2cConfig i2c2Config =
{
    .device                = IIC3_DEVICE,
    .sclPin                = I2C3_SCL,
    .sdaPin                = I2C3_SDA,
    .speedKhz              = I2C3_SPEED_KHZ_DEFAULT,
    .addressSize           = I2C3_ADDR_SIZE_DEFAULT,
    .errorHandler          = &commonI2CErrorHandler,
};

static u8 WM8325protocolWrite[] =
{
    S_ADDR_WR,
    R_ADDR_H,
    R_ADDR_L,
    DATAW,
    LOOP_MINUS_1
};

static u8 WM8325protocolRead[] =
{
    S_ADDR_WR,
    R_ADDR_H,
    R_ADDR_L,
    S_ADDR_RD,
    DATAR,
    LOOP_MINUS_1
};


void InitGpioEth(u8 invertGtxClk)
{
    DrvGpioSetMode(IO_TX_EN , D_GPIO_MODE_4);
    DrvGpioSetMode(IO_TXER  , D_GPIO_MODE_4);
    DrvGpioSetMode(IO_TXD_0 , D_GPIO_MODE_4);
    DrvGpioSetMode(IO_TXD_1 , D_GPIO_MODE_1);
    DrvGpioSetMode(IO_TXD_2 , D_GPIO_MODE_1);
    DrvGpioSetMode(IO_TXD_3 , D_GPIO_MODE_1);
    DrvGpioSetMode(IO_TXD_4 , D_GPIO_MODE_1);
    DrvGpioSetMode(IO_TXD_5 , D_GPIO_MODE_1);
    DrvGpioSetMode(IO_TXD_6 , D_GPIO_MODE_1);
    DrvGpioSetMode(IO_TXD_7 , D_GPIO_MODE_1);
    DrvGpioSetMode(IO_125CLK, D_GPIO_MODE_4);

    if(invertGtxClk){
        DrvGpioSetMode(IO_GTXCK , D_GPIO_MODE_1 | D_GPIO_DATA_INV_ON); // best with this INV;
        DrvGpioPadSet(IO_GTXCK, D_GPIO_PAD_DRIVE_2mA);//Drive: 2ma or 6mA allowed only, using 2mA
    }
    else{
        DrvGpioSetMode(IO_GTXCK , D_GPIO_MODE_1);
        DrvGpioPadSet(IO_GTXCK, D_GPIO_PAD_DRIVE_2mA);
    }

    DrvGpioSetMode(IO_TXCLK , D_GPIO_DIR_IN | D_GPIO_MODE_4 );
    DrvGpioSetMode(IO_RXCLK , D_GPIO_DIR_IN | D_GPIO_MODE_4 );
    DrvGpioSetMode(IO_RXDV  , D_GPIO_DIR_IN | D_GPIO_MODE_4);
    DrvGpioSetMode(IO_RXER  , D_GPIO_DIR_IN | D_GPIO_MODE_4);
    DrvGpioSetMode(IO_RXCOL , D_GPIO_DIR_IN | D_GPIO_MODE_4);
    DrvGpioSetMode(IO_RXCRS , D_GPIO_DIR_IN | D_GPIO_MODE_4);
    DrvGpioSetMode(IO_RXD_0 , D_GPIO_DIR_IN | D_GPIO_MODE_1);
    DrvGpioSetMode(IO_RXD_1 , D_GPIO_DIR_IN | D_GPIO_MODE_1);
    DrvGpioSetMode(IO_RXD_2 , D_GPIO_DIR_IN | D_GPIO_MODE_1);
    DrvGpioSetMode(IO_RXD_3 , D_GPIO_DIR_IN | D_GPIO_MODE_1);
    DrvGpioSetMode(IO_RXD_4 , D_GPIO_DIR_IN | D_GPIO_MODE_1);
    DrvGpioSetMode(IO_RXD_5 , D_GPIO_DIR_IN | D_GPIO_MODE_3);
    DrvGpioSetMode(IO_RXD_6 , D_GPIO_DIR_IN | D_GPIO_MODE_3);
    DrvGpioSetMode(IO_RXD_7 , D_GPIO_DIR_IN | D_GPIO_MODE_3);

    DrvGpioSetMode(IO_MDC, D_GPIO_MODE_4);
    DrvGpioSetMode(IO_MDIO, D_GPIO_MODE_4);
}

static u32 commonI2CErrorHandler(I2CM_StatusType i2cCommsError, u32 slaveAddr, u32 regAddr)
{
    printf("ERROR HANDLER\n");

    slaveAddr = slaveAddr;
    regAddr = regAddr;

    return i2cCommsError; // Because we haven't really handled the error, pass it back to the caller
}

void EthPHYHWReset(void)
{
    unsigned int myVal;
    u8 bytes[2];
    I2CM_Device i2c3Handle;

    DrvI2cMInitFromConfig(&i2c3Handle, &i2c2Config);    // setup i2c module

    //Setting output mode for GPIO pin 8 (Ethernet PHY reset)
    // and set MODE1+2 ( WM8325 Pin 4 ) to 0 for wakeup
    DrvI2cMTransaction(&i2c3Handle, WM8325_I2C, 0x403F, WM8325protocolRead, bytes, 2);
    myVal=(bytes[0]<<8) | bytes[1];
    myVal=0x0480; // output mode ...
    bytes[0]=(myVal & 0xFF00) >> 8;
    bytes[1]=(myVal & 0xFF);
    DrvI2cMTransaction(&i2c3Handle, WM8325_I2C, 0x403F, WM8325protocolWrite, bytes, 2); // ... for gpio 8
    DrvI2cMTransaction(&i2c3Handle, WM8325_I2C, 0x403B, WM8325protocolWrite, bytes, 2); // ... for gpio 4

    //Make sure ETH is in reset
    DrvI2cMTransaction(&i2c3Handle, WM8325_I2C, 0x400C,WM8325protocolRead, bytes, 2);
    myVal=(bytes[0]<<8) | bytes[1];
    SET_BIT(myVal, 3, 0); // GPIO 4 = 0
    SET_BIT(myVal, 7, 0); // GPIO 8 = 0
    bytes[0]=(myVal & 0xFF00) >> 8;
    bytes[1]=(myVal & 0xFF);
    DrvI2cMTransaction(&i2c3Handle, WM8325_I2C, 0x400C, WM8325protocolWrite, bytes, 2);

    // Wait for reset..
    usleep(500 * 1000);

    //Deassert HW reset, but keep ETH_BOOT low
    DrvI2cMTransaction(&i2c3Handle, WM8325_I2C, 0x400C, WM8325protocolRead, bytes, 2);
    myVal=(bytes[0]<<8) | bytes[1];
    SET_BIT(myVal, 7, 1); // GPIO 8 = 1
    bytes[0]=(myVal & 0xFF00) >> 8;
    bytes[1]=(myVal & 0xFF);
    DrvI2cMTransaction(&i2c3Handle, WM8325_I2C, 0x400C, WM8325protocolWrite, bytes, 2);

    // Wait for init..
    // TODO: put back the ETH_BOOT GPIO in input mode (shared with ETH RX pins, but through a resistor)
    usleep(100 * 1000);
}

int NetInit(const XeyePcbType pcb_type, const std::string& ip_addr, \
            const std::string& net_mask, const std::string& gate_way) {
    int res;
    rtems_greth_gbit_hw_params params;
    // Warning: Only XEYE 20 init Reset and Xeye-face will reset eth chip by RC
    if (pcb_type == XEYE_20) {
        EthPHYHWReset();
    }
    InitGpioEth(INVERT_GTX_CLK_CFG);

    params.priority = ETH_ISR_PRIORITY;
    params.txcount = TDA_COUNT;
    params.rxcount = RDA_COUNT;
    params.gbit_check = FALSE;
    params.read_function = NULL;
    params.write_function = NULL;
    if (!ip_addr.empty()) {
        if (net_mask.empty()) {
            mvLog(MVLOG_ERROR, "if you provide ip address, you must provide net mask at the same time");
            return false;
        }
        ifconfig.ip_address = ip_addr.c_str();
        ifconfig.ip_netmask = net_mask.c_str();
        rtems_bsdnet_config.bootp = NULL;
        mvLog(MVLOG_INFO, "Configured local ip address: %s, net mask: %s", ip_addr.c_str(), net_mask.c_str());
    }

    if (!gate_way.empty()) {
        rtems_bsdnet_config.gateway = gate_way.c_str();
        mvLog(MVLOG_INFO, "Also configured gate way to : %s", gate_way.c_str());
    }
    // these name server are used to resovle domain name to ip address
    // DO NOT change it.
    // assume public name server ip
    rtems_bsdnet_config.name_server[0] = "180.76.76.76";
    // ALI public name server ip
    rtems_bsdnet_config.name_server[1] = "223.5.5.5";
    rtems_bsdnet_config.name_server[2] = "223.6.6.6";
    res = rtems_leon_greth_gbit_driver_setup(&params);
    assert(res == RTEMS_SUCCESSFUL);
    while(res = rtems_bsdnet_initialize_network()){
	      usleep(1000*1000*5);
    }
    assert(res == RTEMS_SUCCESSFUL);
    return res;
}
