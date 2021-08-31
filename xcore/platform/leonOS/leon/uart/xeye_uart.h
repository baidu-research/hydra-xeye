/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _XEYE_UART_H_
#define _XEYE_UART_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <registersMyriad.h>
#include <DrvGpioMa2x5x.h>
#include <DrvRegUtils.h>
#include <DrvTimer.h>
#include "DrvCpr.h"
#include "DrvUartDefines.h"
#include "DrvUart.h"
#include "OsDrvUart.h"

#ifdef  __cplusplus  
extern "C" {  
#endif

#define UART_BAURD_RATE 115200

void xeye_drv_uart_gpio_cfg();
int  xeye_drv_uart_init(u32 baudrate);
void xeye_drv_uart_echo(void);
int xeye_os_uart_init(u32 baudrate);
int xeye_os_uart_open(void);
int xeye_os_uart_close(int fd);
int xeye_os_uart_read(int fd, uint8_t * const buf, int max_len);
int xeye_os_uart_write(int fd, uint8_t const *buf, int len);
int xeye_os_uart_config_sw_fifo(int fd);
int xeye_os_uart_config_baudrate(s32 fd, u32 baudrate);
int xeye_os_uart_config_loopback(s32 fd, uartIoctls_t ctl);

#ifdef  __cplusplus  
}
#endif
#endif  // _XEYE_UART_H_
