/*

Module: videodemo_device.h

Function:
	MCCI USB Data Pump(tm) structure layouts, generated automatically
	from leon/videodemo_device.urc by usbrc, version V3.47a (Nov 23 2017)

Copyright:
	Copyright 1997-2015, MCCI Corporation

	Distribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	1. Redistribution of source code must retain the above copyright
	   notice, this list of conditions and the following disclaimer.
	2. MCCI's name may not be used to endorse or promote products
	   derived from this software without specific prior written
	   permission.

Disclaimer:
	THIS FILE IS PROVIDED BY MCCI "AS IS" AND ANY EXPRESS OR
	IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED.  IN NO EVENT SHALL MCCI BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
	GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
	THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Generated:
	Wednesday, June 5, 2019   1:55 pm

*/

#ifndef _VIDEODEMO_DEVICE_H_
#define _VIDEODEMO_DEVICE_H_

#ifndef _USBPUMP_H_
# include "usbpump.h"
#endif /* _USBPUMP_H_ */

/* start with the chip support structures */
#include "xdcdkern.h"

/*
|| Next, define parameters based on URC info.
*/
#define	UDEVICE_VIDEODEMO_TOTALNUMCONFIGS		3	/* number of USB configurations */
#define	UDEVICE_VIDEODEMO_TOTALNUMINTERFACESETS	6	/* number of interfaces (all configs) */
#define	UDEVICE_VIDEODEMO_TOTALNUMINTERFACES	6	/* number of alt settings (all configs) */
#define	UDEVICE_VIDEODEMO_TOTALNUMPIPES		5	/* number of pipes (all configs) */

#define	UDEVICE_VIDEODEMO_HSTOTALNUMCONFIGS		1	/* number of USB configurations (high speed only) */
#define	UDEVICE_VIDEODEMO_HSTOTALNUMINTERFACESETS	2	/* number of interfaces (all configs) */
#define	UDEVICE_VIDEODEMO_HSTOTALNUMINTERFACES	2	/* number of alt settings (all configs) */
#define	UDEVICE_VIDEODEMO_HSTOTALNUMPIPES		1	/* number of pipes (all configs, but not ep0) */


/*
|| Number of endpoints in endpoint structure, from chip info.
*/
#define	UDEVICE_VIDEODEMO_NUMENDPOINTS		32	/* number of endpoints */


/*
|| The following data structure is the UDEVICE for this application of
|| the datapump.
*/
typedef struct TTUSB_UDEVICE_VIDEODEMO
	{
	USBPUMP_XDCD_HDR;
	UCONFIG	udev_cfg[UDEVICE_VIDEODEMO_TOTALNUMCONFIGS];
	UINTERFACESET	udev_ifcset[UDEVICE_VIDEODEMO_TOTALNUMINTERFACESETS];
	UINTERFACE	udev_ifc[UDEVICE_VIDEODEMO_TOTALNUMINTERFACES];
	UPIPE		udev_pipes[UDEVICE_VIDEODEMO_TOTALNUMPIPES];
	USBPUMP_ENDPOINT_XDCD	udev_endpoints[UDEVICE_VIDEODEMO_NUMENDPOINTS];
	} UDEVICE_VIDEODEMO, *PUDEVICE_VIDEODEMO;


/*
|| Define the prototype for the function we will compile:
*/
UDEVICE_INITFN usbrc_UdeviceVideoDemo_init;

/**** end of videodemo_device.h ****/
#endif /* _VIDEODEMO_DEVICE_H_ */
