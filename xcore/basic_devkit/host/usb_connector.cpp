/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "usb_connector.h"
#include "xeye_message_data_type.h"
#include "xeye_usb.h"
#include "mv_types.h"
#include "debug_def.h"

#include <assert.h>
#include <time.h>

bool UsbConnector::_s_opened = false;

UsbConnector::UsbConnector() {
}

UsbConnector::~UsbConnector() {
    uninit();
}

int UsbConnector::init() {
    int ret = vsc_open();
    if (0 != ret) {
        printf("vsc_open failed. ret:%d\n", ret);
    } else {
        UsbConnector::_s_opened = true;
    }
    return ret;
}

int UsbConnector::send(uint8_t* data, uint32_t size, uint32_t timeout_ms) {
    if (NULL == data || 0 == size) {
        printf("UsbConnector::send invalid parameters.\n");
        return -1;
    }

    int ret = vsc_write(data, size, timeout_ms);
    if (0 != ret) {
        if (1 == ret) {
            DEBUG_PRINT("usb send timeout. size:%d\n", (int)size);
        } else {
            printf("vsc_write failed. ret:%d\n", ret);
        }
    }
    return ret;
}

int UsbConnector::receive(uint8_t* data, uint32_t size, uint32_t timeout_ms) {
    if (NULL == data || 0 == size) {
        printf("UsbConnector::receive invalid parameters.\n");
        return -1;
    }

    int ret = vsc_read(data, size, timeout_ms);
    if (0 != ret) {
        if (1 == ret) {
            DEBUG_PRINT("usb receive timeout. size:%d\n", (int)size);
        } else {
            printf("vsc_read failed. ret:%d\n", ret);
        }
    }
    return ret;
}

int UsbConnector::uninit() {
    if (!UsbConnector::_s_opened) {
        return -1;
    }
    int ret = vsc_close();
    if (0 != ret) {
        printf("vsc_close failed. ret:%d\n", ret);
    } else {
        UsbConnector::_s_opened = false;
    }
    return ret;
}
