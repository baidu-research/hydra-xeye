#ifndef _SUNNY_USB_API_H
#define _SUNNY_USB_API_H

#ifdef __cplusplus
extern "C" {
#endif

int UsbInit(void);
int UsbReceive(uint8_t* data, uint32_t size, uint32_t timeout_ms);
int UsbSend(uint8_t* data, uint32_t size, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif
#endif
