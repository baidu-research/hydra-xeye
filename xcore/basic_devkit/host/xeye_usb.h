//     __    __  _____  __    __  _____ 
//     \ \  / / | ____| \ \  / / |  ___|
//      \ \/ /  | |__    \ \/ /  | |__  
//       }  {   |  __|    \  /   |  __| 
//      / /\ \  | |___    / /    | |___ 
//     /_/  \_\ |_____|  /_/     |_____|
//                                      
// >File:     xeye_usb.h
// >Commnet:  
// >Ver:      v1.0
// >History:  
// >Created by zq on:2018年08月28日 星期二 14时32分25秒
//
#ifndef _XEYE_USB_H
#define _XEYE_USB_H

#include <libusb-1.0/libusb.h>

#define VENDORID 0x040E
#define DEV_VID 0x040E
#define DEV_PID 0xF63B

int get_vsc_device(libusb_device *dev);
void print_vsc_devices(libusb_device **devs);

int vsc_open();
int vsc_close();
int vsc_read(unsigned char *buf, int size, int timeout_ms);
int vsc_write(unsigned char *buf, int size, int timeout_ms);

#endif
