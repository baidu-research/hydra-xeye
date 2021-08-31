/*

Module: videodemo_device.c

Function:
	USB peripheral device descriptor data, generated automatically
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

/* make sure we get the V5/ V4/ V3 structures */
#ifndef __USBRCTAB_WANT_V3
# define __USBRCTAB_WANT_V3 1
#endif /* __USBRCTAB_WANT_V3 */

#ifndef __USBRCTAB_WANT_V4
# define __USBRCTAB_WANT_V4 1
#endif /* __USBRCTAB_WANT_V4 */

#ifndef __USBRCTAB_WANT_V5
# define __USBRCTAB_WANT_V5 1
#endif /* __USBRCTAB_WANT_V5 */

#include "videodemo_device.h"

/* beginning of resource data */
// default is 1080p
unsigned char gk_UsbResourceData[] =
	{
	18, 1, 16, 2, 239, 2, 1, 64,
	14, 4, 8, 244, 16, 1, 1, 2,
	3, 1, 9, 2, 177, 0, 2, 1,
	0, 192, 20, 8, 11, 0, 2, 14,
	3, 0, 0, 9, 4, 0, 0, 0,
	14, 1, 0, 4, 13, 36, 1, 0,
	1, 50, 0, 232, 3, 0, 0, 1,
	1, 17, 36, 2, 1, 1, 2, 0,
	0, 0, 0, 0, 0, 0, 0, 2,
	0, 0, 9, 36, 3, 2, 1, 1,
	0, 3, 0, 11, 36, 5, 3, 1,
	0, 0, 2, 127, 29, 0, 9, 4,
	1, 0, 1, 14, 2, 0, 5, 14,
	36, 1, 1, 85, 0, 129, 0, 2,
	0, 0, 0, 1, 0, 27, 36, 4,
	1, 1, 89, 85, 89, 50, 0, 0,
	16, 0, 128, 0, 0, 170, 0, 56,
	155, 113, 16, 1, 0, 0, 0, 0,
	38, 36, 5, 1, 2, 128, 7, 56,
	4, 0, 128, 81, 1, 0, 128, 81,
	1, 0, 0, 27, 0, 64, 66, 15,
	0, 0, 64, 66, 15, 0, 64, 66,
	15, 0, 0, 0, 0, 0, 6, 36,
	13, 1, 1, 1, 7, 5, 129, 2,
	64, 0, 0, 18, 1, 16, 2, 239,
	2, 1, 64, 14, 4, 8, 244, 16,
	1, 1, 2, 3, 1, 9, 2, 177,
	0, 2, 1, 0, 192, 20, 8, 11,
	0, 2, 14, 3, 0, 0, 9, 4,
	0, 0, 0, 14, 1, 0, 4, 13,
	36, 1, 0, 1, 50, 0, 232, 3,
	0, 0, 1, 1, 17, 36, 2, 1,
	1, 2, 0, 0, 0, 0, 0, 0,
	0, 0, 2, 0, 0, 9, 36, 3,
	2, 1, 1, 0, 3, 0, 11, 36,
	5, 3, 1, 0, 0, 2, 127, 29,
	0, 9, 4, 1, 0, 1, 14, 2,
	0, 5, 14, 36, 1, 1, 85, 0,
	129, 0, 2, 0, 0, 0, 1, 0,
	27, 36, 4, 1, 1, 89, 85, 89,
	50, 0, 0, 16, 0, 128, 0, 0,
	170, 0, 56, 155, 113, 16, 1, 0,
	0, 0, 0, 38, 36, 5, 1, 2,
	128, 7, 56, 4, 0, 128, 81, 1,
	0, 128, 81, 1, 0, 0, 27, 0,
	64, 66, 15, 0, 0, 64, 66, 15,
	0, 64, 66, 15, 0, 0, 0, 0,
	0, 6, 36, 13, 1, 1, 1, 7,
	5, 129, 2, 0, 2, 0, 18, 1,
	16, 3, 239, 2, 1, 9, 14, 4,
	8, 244, 16, 1, 1, 2, 3, 1,
	9, 2, 183, 0, 2, 1, 0, 192,
	5, 8, 11, 0, 2, 14, 3, 0,
	0, 9, 4, 0, 0, 0, 14, 1,
	0, 4, 13, 36, 1, 0, 1, 50,
	0, 232, 3, 0, 0, 1, 1, 17,
	36, 2, 1, 1, 2, 0, 0, 0,
	0, 0, 0, 0, 0, 2, 0, 0,
	9, 36, 3, 2, 1, 1, 0, 3,
	0, 11, 36, 5, 3, 1, 0, 0,
	2, 127, 29, 0, 9, 4, 1, 0,
	1, 14, 2, 0, 5, 14, 36, 1,
	1, 85, 0, 129, 0, 2, 0, 0,
	0, 1, 0, 27, 36, 4, 1, 1,
	89, 85, 89, 50, 0, 0, 16, 0,
	128, 0, 0, 170, 0, 56, 155, 113,
	16, 1, 0, 0, 0, 0, 38, 36,
	5, 1, 2, 128, 7, 56, 4, 0,
	0, 167, 118, 0, 0, 167, 118, 0,
	0, 27, 0, 10, 139, 2, 0, 0,
	10, 139, 2, 0, 10, 139, 2, 0,
	0, 0, 0, 0, 6, 36, 13, 1,
	1, 1, 7, 5, 129, 2, 0, 4,
	0, 6, 48, 15, 0, 0, 0, 4,
	3, 9, 4, 34, 3, 77, 0, 67,
	0, 67, 0, 73, 0, 32, 0, 67,
	0, 111, 0, 114, 0, 112, 0, 111,
	0, 114, 0, 97, 0, 116, 0, 105,
	0, 111, 0, 110, 0, 56, 3, 77,
	0, 67, 0, 67, 0, 73, 0, 40,
	0, 114, 0, 41, 0, 32, 0, 86,
	0, 105, 0, 100, 0, 101, 0, 111,
	0, 32, 0, 68, 0, 101, 0, 109,
	0, 111, 0, 32, 0, 70, 0, 105,
	0, 114, 0, 109, 0, 119, 0, 97,
	0, 114, 0, 101, 0, 12, 3, 68,
	0, 85, 0, 77, 0, 77, 0, 89,
	0, 38, 3, 77, 0, 67, 0, 67,
	0, 73, 0, 40, 0, 114, 0, 41,
	0, 32, 0, 86, 0, 105, 0, 100,
	0, 101, 0, 111, 0, 32, 0, 68,
	0, 101, 0, 109, 0, 111, 0, 40,
	3, 86, 0, 105, 0, 100, 0, 101,
	0, 111, 0, 32, 0, 73, 0, 110,
	0, 32, 0, 40, 0, 100, 0, 105,
	0, 115, 0, 97, 0, 98, 0, 108,
	0, 101, 0, 100, 0, 41, 0, 5,
	15, 22, 0, 2, 7, 16, 2, 6,
	0, 0, 0, 10, 16, 3, 0, 14,
	0, 1, 10, 32, 0
	};

/* beginning of descriptor table */
static RESPTRS gk_UsbResourceTable_1[] =
	{
	RESPTRVAL(gk_UsbResourceData, 0)
	};  /* end of gk_UsbResourceTable_1 */

/* beginning of descriptor table */
static RESPTRS gk_UsbResourceTable_2[] =
	{
	RESPTRVAL(gk_UsbResourceData, 18)
	};  /* end of gk_UsbResourceTable_2 */

/* beginning of descriptor table */
static RESPTRS gk_UsbResourceTable_3[] =
	{
	RESPTRVAL(gk_UsbResourceData, 195)
	};  /* end of gk_UsbResourceTable_3 */

/* beginning of descriptor table */
static RESPTRS gk_UsbResourceTable_4[] =
	{
	RESPTRVAL(gk_UsbResourceData, 213)
	};  /* end of gk_UsbResourceTable_4 */

/* beginning of descriptor table */
static RESPTRS gk_UsbResourceTable_5[] =
	{
	RESPTRVAL(gk_UsbResourceData, 390)
	};  /* end of gk_UsbResourceTable_5 */

/* beginning of descriptor table */
static RESPTRS gk_UsbResourceTable_6[] =
	{
	RESPTRVAL(gk_UsbResourceData, 408)
	};  /* end of gk_UsbResourceTable_6 */

/* beginning of descriptor table */
static RESPTRS gk_UsbResourceTable_7[] =
	{
	RESPTRVAL(gk_UsbResourceData, 591),
	RESPTRVAL(gk_UsbResourceData, 595),
	RESPTRVAL(gk_UsbResourceData, 629),
	RESPTRVAL(gk_UsbResourceData, 685),
	RESPTRVAL(gk_UsbResourceData, 697),
	RESPTRVAL(gk_UsbResourceData, 735)
	};  /* end of gk_UsbResourceTable_7 */


/* beginning of language ID data */
static LANGIDPTRS gk_UsbLangidTable_8[] =
	{
	LANGPTRVAL(0x0000, 6, gk_UsbResourceTable_7),
	LANGPTRVAL(0x0409, 6, gk_UsbResourceTable_7)
	};  /* end of gk_UsbLangidTable_8 */
/* beginning of descriptor table */
static RESPTRS gk_UsbResourceTable_9[] =
	{
	RESPTRVAL(gk_UsbResourceData, 775)
	};  /* end of gk_UsbResourceTable_9 */

/* beginning of speed specific descriptor table */
static USBRCTAB_SPEED_DEPENDENT_DESCRIPTORS_T gk_UsbResourceTable_10 =
	USBRCTAB_SPEED_DEPENDENT_DESCRIPTORS_INIT_V1( \
		/* devtbl, len devtbl */	gk_UsbResourceTable_1, 1, \
		/* contbl, len contbl */	gk_UsbResourceTable_2, 1, \
		/* bostbl, len bostbl */	gk_UsbResourceTable_9, 1, \
		/* securitytbl, len securitytbl */	0, 0 \
	);

/* beginning of speed specific descriptor table */
static USBRCTAB_SPEED_DEPENDENT_DESCRIPTORS_T gk_UsbResourceTable_11 =
	USBRCTAB_SPEED_DEPENDENT_DESCRIPTORS_INIT_V1( \
		/* devtbl, len devtbl */	gk_UsbResourceTable_3, 1, \
		/* contbl, len contbl */	gk_UsbResourceTable_4, 1, \
		/* bostbl, len bostbl */	gk_UsbResourceTable_9, 1, \
		/* securitytbl, len securitytbl */	0, 0 \
	);

/* beginning of speed specific descriptor table */
static USBRCTAB_SPEED_DEPENDENT_DESCRIPTORS_T gk_UsbResourceTable_12 =
	USBRCTAB_SPEED_DEPENDENT_DESCRIPTORS_INIT_V1( \
		/* devtbl, len devtbl */	gk_UsbResourceTable_5, 1, \
		/* contbl, len contbl */	gk_UsbResourceTable_6, 1, \
		/* bostbl, len bostbl */	gk_UsbResourceTable_9, 1, \
		/* securitytbl, len securitytbl */	0, 0 \
	);


/**** init function declaration ****/
USBRCTAB_DECLARE_DCD_INIT_FN_NOCLASS(usbrc_UdeviceVideoDemo_init)

/**** filter function declarations ****/
USBRCTAB_DECLARE_GETDESCRIPTOR_FN(UsbPumpFilter_MultiConfigModalDescriptors)

/**** bit table declarations ****/
USBRCTAB_DECLARE_BITMAP(static , gk_UsbStringIdBitmap_13) = { 8 };
USBRCTAB_DECLARE_BITMAP(static , gk_UsbStringIdBitmap_14) = { 8 };

/**** the root table ****/
ROOTTABLE_V5_WITH_SEMI( \
	/* name */			gk_UsbDescriptorRoot, \
	/* init-device-fn */		usbrc_UdeviceVideoDemo_init, \
	/* UDEVICE size */		sizeof(UDEVICE_VIDEODEMO), \
	/* devtbl, len devtbl */	gk_UsbResourceTable_1, 1, \
	/* cfgtbl, len cfgtbl */	gk_UsbResourceTable_2, 1, \
	/* hsdevtbl, len hsdevtbl */	gk_UsbResourceTable_3, 1, \
	/* hscfgtbl, len hscfgtbl */	gk_UsbResourceTable_4, 1, \
	/* strtbl, #langs */		gk_UsbLangidTable_8, 2, \
	/* others, len others */	0, 0, \
	/* get-descriptor-fn */		UsbPumpFilter_MultiConfigModalDescriptors, \
	/* set-descriptor-fn */		0, \
	/* ext bitmap, len */		gk_UsbStringIdBitmap_13, 1, \
	/* r/o bitmap, len */		gk_UsbStringIdBitmap_14, 1, \
	/* pUdeviceSwitch */	&gk_UsbPumpXdcd_switch, \
	/* pFsDescSet */	&gk_UsbResourceTable_10, \
	/* pHsDescSet */	&gk_UsbResourceTable_11, \
	/* pWirelessDescSet */	0, \
	/* pSsDescSet */	&gk_UsbResourceTable_12 \
	)


/***** end of generated data *****/
/*

Module: videodemo_device.c

Function:
	MCCI USB data pump structure layouts, generated automatically
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



#include "videodemo_device.h"
#include "descfns.h"


VOID usbrc_UdeviceVideoDemo_init(
	UDEVICE *pDev,
	size_t p_size
	)
	{
	UDEVICE_VIDEODEMO *p = (UDEVICE_VIDEODEMO *) pDev;
	UCONFIG *pCurCfg;
	UINTERFACESET *pCurIfcSet;
	UINTERFACE *pCurIfc;
	UPIPE *pCurPipe;
	int iEndpoint;

	/* Check for structure mismatches */
	if (p == NULL || p_size != sizeof(*p))
		UHIL_swc(UHILERR_INIT_FAIL);

	/* standard object initialization */
	UsbPumpObject_Init(
		&p->udev_Header,
		p->udev_pPlatform->upf_Header.pClassParent,
		UDEVICE_TAG,
		p_size,
		UDEVICE_NAME("UDEVICE_VIDEODEMO" "." "xdcd"),
		&p->udev_pPlatform->upf_Header,
		UsbPumpDeviceI_Ioctl
		);

	/* Initialization of the local variables */
	pCurCfg = NULL;
	pCurIfcSet = NULL;
	pCurIfc = NULL;
	pCurPipe = NULL;

	/* Initialize the Device Header */
	p->udev_pAllConfigs = &(p->udev_cfg[0]);
	p->udev_pConfigs = &(p->udev_cfg[0]);
	p->udev_pEndpoints = (UENDPOINT *) &(p->udev_endpoints[0]);
	p->udev_bSupportedSpeeds = USBPUMP_DEVICE_SPEED_MASK_FULL | USBPUMP_DEVICE_SPEED_MASK_HIGH | USBPUMP_DEVICE_SPEED_MASK_SUPER;
	p->udev_bCurrentSpeed = USBPUMP_DEVICE_SPEED_FULL;
	p->udev_pipes[0].upipe_pEndpoint = (UENDPOINT *) &p->udev_endpoints[0];
	p->udev_pipes[0].upipe_pInterface       = &p->udev_ctlifc;
	p->udev_pipes[0].upipe_wMaxPacketSize   = 64;
	p->udev_pipes[0].upipe_bmAttributes     = USB_bmAttr_CONTROL;
	p->udev_pipes[0].upipe_bEndpointAddress = 0;
	p->udev_pipes[0].upipe_pEpDesc          = &gk_UsbPump_DummyEp0Desc_64;

	p->udev_pipes[1].upipe_pEndpoint = (UENDPOINT *) &p->udev_endpoints[1];
	p->udev_pipes[1].upipe_pInterface       = &p->udev_ctlifc;
	p->udev_pipes[1].upipe_wMaxPacketSize   = 64;
	p->udev_pipes[1].upipe_bmAttributes     = USB_bmAttr_CONTROL;
	p->udev_pipes[1].upipe_bEndpointAddress = USB_bEndpointAddress_IN;
	p->udev_pipes[1].upipe_pEpDesc          = &gk_UsbPump_DummyEp0Desc_64;

	p->udev_ctlifc.uifc_pInterfaceSet = &p->udev_ctlifcset;
	p->udev_ctlifc.uifc_bNumPipes = 2;
	p->udev_ctlifc.uifc_pPipes = p->udev_pipes;
	p->udev_ctlifc.uifc_pIfcDesc = &gk_UsbPump_DummyIfc0Desc;

	p->udev_ctlifcset.uifcset_pInterfaces = &p->udev_ctlifc;
	p->udev_ctlifcset.uifcset_pCurrent    = &p->udev_ctlifc;
	p->udev_ctlifcset.uifcset_pConfig     = &p->udev_cfg[0];

	p->udev_pCtrlIfcset = &p->udev_ctlifcset;
	p->udev_wNumAllConfigs = UDEVICE_VIDEODEMO_TOTALNUMCONFIGS;
	p->udev_bNumConfigs = 1;
	p->udev_bNumHSConfigs = UDEVICE_VIDEODEMO_HSTOTALNUMCONFIGS;
	p->udev_bNumEndpoints = UDEVICE_VIDEODEMO_NUMENDPOINTS;
	p->udev_bNumAllInterfaceSets = UDEVICE_VIDEODEMO_TOTALNUMINTERFACESETS;
	p->udev_bNumAllInterfaces = UDEVICE_VIDEODEMO_TOTALNUMINTERFACES;
	p->udev_wNumAllPipes = UDEVICE_VIDEODEMO_TOTALNUMPIPES;
	p->udev_pvAllInterfaceSets = &(p->udev_ifcset[0]);
	p->udev_pvAllInterfaces = &(p->udev_ifc[0]);
	p->udev_pvAllPipes = &(p->udev_pipes[0]);
	p->udev_pDevDesc = (CONST USBIF_DEVDESC_WIRE *) &gk_UsbResourceData[0];

	/* make sure all endpoints start out initialized */
	for (iEndpoint = 2; iEndpoint < UDEVICE_VIDEODEMO_NUMENDPOINTS; ++iEndpoint)
		xdcd_EpioInit((USBPUMP_DEVICE_XDCD *) p, NULL, &p->udev_endpoints[iEndpoint], (UCHAR) iEndpoint);

	/* Initialize the default pipes and endpoints */
	xdcd_EpioInit((USBPUMP_DEVICE_XDCD *) p, &p->udev_pipes[0], &p->udev_endpoints[0], 0);
	xdcd_EpioInit((USBPUMP_DEVICE_XDCD *) p, &p->udev_pipes[1], &p->udev_endpoints[1], 1);

	/* Now Processing Configuration 1 LS/FS */
	pCurCfg = &(p->udev_cfg[0]);
	pCurCfg->ucfg_pDevice = pDev;
	pCurCfg->ucfg_bNumInterfaces = 2;
	pCurCfg->ucfg_pInterfaceSets = & (p->udev_ifcset[0]);
	UCONFIG_SETSIZE(pCurCfg, sizeof(*pCurCfg)); 
	pCurCfg->ucfg_pCfgDesc = (CONST USBIF_CFGDESC_WIRE *) &gk_UsbResourceData[18];

	/* ++++ Begin initializing Interface set 0 in Cfg 1 LS/FS */
	pCurIfcSet = &(p->udev_ifcset[0]);
	pCurIfcSet->uifcset_pConfig = pCurCfg;
	pCurIfcSet->uifcset_pInterfaces = &(p->udev_ifc[0]);
	UINTERFACESET_SETSIZE(pCurIfcSet, sizeof(*pCurIfcSet));

	/* Now processing Ifc 0 in Ifc set 0 of Cfg 1 LS/FS */
	pCurIfc = &(p->udev_ifc[0]);
	pCurIfc->uifc_pInterfaceSet = pCurIfcSet;
	pCurIfc->uifc_pPipes = &(p->udev_pipes[2]);
	pCurIfc->uifc_pIfcDesc = (CONST USBIF_IFCDESC_WIRE *) &gk_UsbResourceData[35];
	pCurIfc->uifc_bNumPipes = 0;
	pCurIfc->uifc_bAlternateSetting = 0;
	UINTERFACE_SETSIZE(pCurIfc, sizeof(*pCurIfc)); 
	pCurIfcSet->uifcset_bNumAltSettings = 1;
	/* ---- Done with Interface Set 0 of Cfg 1 LS/FS ---- */


	/* ++++ Begin initializing Interface set 1 in Cfg 1 LS/FS */
	pCurIfcSet = &(p->udev_ifcset[1]);
	pCurIfcSet->uifcset_pConfig = pCurCfg;
	pCurIfcSet->uifcset_pInterfaces = &(p->udev_ifc[1]);
	UINTERFACESET_SETSIZE(pCurIfcSet, sizeof(*pCurIfcSet));

	/* Now processing Ifc 0 in Ifc set 1 of Cfg 1 LS/FS */
	pCurIfc = &(p->udev_ifc[1]);
	pCurIfc->uifc_pInterfaceSet = pCurIfcSet;
	pCurIfc->uifc_pPipes = &(p->udev_pipes[2]);
	pCurIfc->uifc_pIfcDesc = (CONST USBIF_IFCDESC_WIRE *) &gk_UsbResourceData[94];
	pCurIfc->uifc_bNumPipes = 1;
	pCurIfc->uifc_bAlternateSetting = 0;
	UINTERFACE_SETSIZE(pCurIfc, sizeof(*pCurIfc)); 

	/* Initialize Pipe 0 in Ifc 0 in Ifcset 1 of Cfg 1 LS/FS */
	pCurPipe = &(p->udev_pipes[2]);
	pCurPipe->upipe_pInterface = pCurIfc;
	pCurPipe->upipe_pEpDesc = (CONST USBIF_EPDESC_WIRE *) &gk_UsbResourceData[188];
	pCurPipe->upipe_bEndpointAddress = 0x81;
	pCurPipe->upipe_pEndpoint = (UENDPOINT *) &(p->udev_endpoints[3]);
	pCurPipe->upipe_wMaxPacketSize = 64;
	pCurPipe->upipe_bmAttributes = USB_bmAttr_BULK;
	pCurIfcSet->uifcset_bNumAltSettings = 1;
	/* ---- Done with Interface Set 1 of Cfg 1 LS/FS ---- */


	/* Now Processing Configuration 1 HS */
	pCurCfg = &(p->udev_cfg[1]);
	pCurCfg->ucfg_pDevice = pDev;
	pCurCfg->ucfg_bNumInterfaces = 2;
	pCurCfg->ucfg_pInterfaceSets = & (p->udev_ifcset[2]);
	UCONFIG_SETSIZE(pCurCfg, sizeof(*pCurCfg)); 
	pCurCfg->ucfg_pCfgDesc = (CONST USBIF_CFGDESC_WIRE *) &gk_UsbResourceData[213];

	/* ++++ Begin initializing Interface set 0 in Cfg 1 HS */
	pCurIfcSet = &(p->udev_ifcset[2]);
	pCurIfcSet->uifcset_pConfig = pCurCfg;
	pCurIfcSet->uifcset_pInterfaces = &(p->udev_ifc[2]);
	UINTERFACESET_SETSIZE(pCurIfcSet, sizeof(*pCurIfcSet));

	/* Now processing Ifc 0 in Ifc set 0 of Cfg 1 HS */
	pCurIfc = &(p->udev_ifc[2]);
	pCurIfc->uifc_pInterfaceSet = pCurIfcSet;
	pCurIfc->uifc_pPipes = &(p->udev_pipes[3]);
	pCurIfc->uifc_pIfcDesc = (CONST USBIF_IFCDESC_WIRE *) &gk_UsbResourceData[230];
	pCurIfc->uifc_bNumPipes = 0;
	pCurIfc->uifc_bAlternateSetting = 0;
	UINTERFACE_SETSIZE(pCurIfc, sizeof(*pCurIfc)); 
	pCurIfcSet->uifcset_bNumAltSettings = 1;
	/* ---- Done with Interface Set 0 of Cfg 1 HS ---- */


	/* ++++ Begin initializing Interface set 1 in Cfg 1 HS */
	pCurIfcSet = &(p->udev_ifcset[3]);
	pCurIfcSet->uifcset_pConfig = pCurCfg;
	pCurIfcSet->uifcset_pInterfaces = &(p->udev_ifc[3]);
	UINTERFACESET_SETSIZE(pCurIfcSet, sizeof(*pCurIfcSet));

	/* Now processing Ifc 0 in Ifc set 1 of Cfg 1 HS */
	pCurIfc = &(p->udev_ifc[3]);
	pCurIfc->uifc_pInterfaceSet = pCurIfcSet;
	pCurIfc->uifc_pPipes = &(p->udev_pipes[3]);
	pCurIfc->uifc_pIfcDesc = (CONST USBIF_IFCDESC_WIRE *) &gk_UsbResourceData[289];
	pCurIfc->uifc_bNumPipes = 1;
	pCurIfc->uifc_bAlternateSetting = 0;
	UINTERFACE_SETSIZE(pCurIfc, sizeof(*pCurIfc)); 

	/* Initialize Pipe 0 in Ifc 0 in Ifcset 1 of Cfg 1 HS */
	pCurPipe = &(p->udev_pipes[3]);
	pCurPipe->upipe_pInterface = pCurIfc;
	pCurPipe->upipe_pEpDesc = (CONST USBIF_EPDESC_WIRE *) &gk_UsbResourceData[383];
	pCurPipe->upipe_bEndpointAddress = 0x81;
	pCurPipe->upipe_pEndpoint = (UENDPOINT *) &(p->udev_endpoints[3]);
	pCurPipe->upipe_wMaxPacketSize = 512;
	pCurPipe->upipe_bmAttributes = USB_bmAttr_BULK;
	pCurIfcSet->uifcset_bNumAltSettings = 1;
	/* ---- Done with Interface Set 1 of Cfg 1 HS ---- */


	/* Now Processing Configuration 1 Super Speed */
	pCurCfg = &(p->udev_cfg[2]);
	pCurCfg->ucfg_pDevice = pDev;
	pCurCfg->ucfg_bNumInterfaces = 2;
	pCurCfg->ucfg_pInterfaceSets = & (p->udev_ifcset[4]);
	UCONFIG_SETSIZE(pCurCfg, sizeof(*pCurCfg)); 
	pCurCfg->ucfg_pCfgDesc = (CONST USBIF_CFGDESC_WIRE *) &gk_UsbResourceData[408];

	/* ++++ Begin initializing Interface set 0 in Cfg 1 Super Speed */
	pCurIfcSet = &(p->udev_ifcset[4]);
	pCurIfcSet->uifcset_pConfig = pCurCfg;
	pCurIfcSet->uifcset_pInterfaces = &(p->udev_ifc[4]);
	UINTERFACESET_SETSIZE(pCurIfcSet, sizeof(*pCurIfcSet));

	/* Now processing Ifc 0 in Ifc set 0 of Cfg 1 Super Speed */
	pCurIfc = &(p->udev_ifc[4]);
	pCurIfc->uifc_pInterfaceSet = pCurIfcSet;
	pCurIfc->uifc_pPipes = &(p->udev_pipes[4]);
	pCurIfc->uifc_pIfcDesc = (CONST USBIF_IFCDESC_WIRE *) &gk_UsbResourceData[425];
	pCurIfc->uifc_bNumPipes = 0;
	pCurIfc->uifc_bAlternateSetting = 0;
	UINTERFACE_SETSIZE(pCurIfc, sizeof(*pCurIfc)); 
	pCurIfcSet->uifcset_bNumAltSettings = 1;
	/* ---- Done with Interface Set 0 of Cfg 1 Super Speed ---- */


	/* ++++ Begin initializing Interface set 1 in Cfg 1 Super Speed */
	pCurIfcSet = &(p->udev_ifcset[5]);
	pCurIfcSet->uifcset_pConfig = pCurCfg;
	pCurIfcSet->uifcset_pInterfaces = &(p->udev_ifc[5]);
	UINTERFACESET_SETSIZE(pCurIfcSet, sizeof(*pCurIfcSet));

	/* Now processing Ifc 0 in Ifc set 1 of Cfg 1 Super Speed */
	pCurIfc = &(p->udev_ifc[5]);
	pCurIfc->uifc_pInterfaceSet = pCurIfcSet;
	pCurIfc->uifc_pPipes = &(p->udev_pipes[4]);
	pCurIfc->uifc_pIfcDesc = (CONST USBIF_IFCDESC_WIRE *) &gk_UsbResourceData[484];
	pCurIfc->uifc_bNumPipes = 1;
	pCurIfc->uifc_bAlternateSetting = 0;
	UINTERFACE_SETSIZE(pCurIfc, sizeof(*pCurIfc)); 

	/* Initialize Pipe 0 in Ifc 0 in Ifcset 1 of Cfg 1 Super Speed */
	pCurPipe = &(p->udev_pipes[4]);
	pCurPipe->upipe_pInterface = pCurIfc;
	pCurPipe->upipe_pEpDesc = (CONST USBIF_EPDESC_WIRE *) &gk_UsbResourceData[578];
	pCurPipe->upipe_bEndpointAddress = 0x81;
	pCurPipe->upipe_pEndpoint = (UENDPOINT *) &(p->udev_endpoints[3]);
	pCurPipe->upipe_wMaxPacketSize = 1024;
	pCurPipe->upipe_bmAttributes = USB_bmAttr_BULK;
	pCurIfcSet->uifcset_bNumAltSettings = 1;
	/* ---- Done with Interface Set 1 of Cfg 1 Super Speed ---- */

	}
/* End of File */
