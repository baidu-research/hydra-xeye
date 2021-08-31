/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_USB_CONNECTOR_H
#define BAIDU_XEYE_USB_CONNECTOR_H

#include <libusb-1.0/libusb.h>
#include <stdint.h>

class UsbConnector {
public:
    UsbConnector();
    ~UsbConnector();

    int init();
    int send(uint8_t* data, uint32_t size, uint32_t timeout_ms);
    int receive(uint8_t* data, uint32_t size, uint32_t timeout_ms);

private:
    int uninit();

    static bool _s_opened;
};

#endif // BAIDU_XEYE_USB_CONNECTOR_H
