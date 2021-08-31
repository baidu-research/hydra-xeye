/*
 * vsc2app_outcall.h
 *
 *  Created on: Aug 31, 2015
 *      Author: mario
 */
#include "usbpump_vsc2app.h"

//#define DUSBPRINT_ENABLE

#ifndef DUSBPRINT_ENABLE
    #define DUSBPRINT(...)
#else
    #define DUSBPRINT(...) printf(__VA_ARGS__)
#endif

VOID
UsbVscAppRead(USBPUMP_VSC2APP_CONTEXT * pSelf, __TMS_UINT32 size, __TMS_CHAR * buff, __TMS_UINT32 endPoint);

VOID
UsbVscAppWrite(USBPUMP_VSC2APP_CONTEXT * pSelf, __TMS_UINT32 size, __TMS_CHAR * buff, __TMS_UINT32 endPoint);
