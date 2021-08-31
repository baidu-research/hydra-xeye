#include "xeye_usb.h"

#include <libusb-1.0/libusb.h>
#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define PACKET_SIZE 512
#define DEV_INTERFACE 0

#define IS_TO_DEVICE   0   /* to device */
#define IS_TO_HOST     0x80   /* to host */
#define IS_BULK        2

int g_out_end_point = 0;
int g_in_end_point = 0;

libusb_device_handle *g_dev_handle;
libusb_context *g_dev_context;

int get_vsc_device(libusb_device *dev) {
    struct libusb_config_descriptor *config = NULL;
    int r = libusb_get_active_config_descriptor(dev, &config);
    if (r < 0) {
        return -1;
    }

    int iface_idx = 0;
    for (iface_idx = 0; iface_idx < config->bNumInterfaces; iface_idx++) {
        const struct libusb_interface *iface = &config->interface[iface_idx];
        int altsetting_idx = 0;

        for (altsetting_idx = 0; altsetting_idx < iface->num_altsetting;
                altsetting_idx++) {
            const struct libusb_interface_descriptor *altsetting =
                &iface->altsetting[altsetting_idx];
            int ep_idx = 0;

            for (ep_idx = 0; ep_idx < altsetting->bNumEndpoints; ep_idx++) {
                const struct libusb_endpoint_descriptor *ep =
                    &altsetting->endpoint[ep_idx];

                if (IS_TO_DEVICE == (ep->bEndpointAddress & 0x80)
                        && IS_BULK == (ep->bmAttributes & 0x03)) {
                    g_out_end_point = ep->bEndpointAddress;
                    printf("outEndPoint:[%x]\r\n", g_out_end_point);
                }
                if (IS_TO_HOST == (ep->bEndpointAddress & 0x80)
                        && IS_BULK == (ep->bmAttributes & 0x03)) {
                    g_in_end_point = ep->bEndpointAddress;
                    printf("inEndPoint:[%x]\r\n", g_in_end_point);
                }
            }
        }

    }

    libusb_free_config_descriptor(config);
    return 0;
}

void print_vsc_devices(libusb_device **devs) {
    libusb_device *dev = NULL;
    int i = 0;
    int j = 0;
    uint8_t path[8];

    while ((dev = devs[i++]) != NULL) {
        struct libusb_device_descriptor desc;
        int r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0) {
            fprintf(stderr, "failed to get device descriptor");
            return;
        }

        printf("%04x:%04x (bus %d, device %d)", desc.idVendor, desc.idProduct,
                libusb_get_bus_number(dev), libusb_get_device_address(dev));

        r = libusb_get_port_numbers(dev, path, sizeof(path));
        if (r > 0) {
            printf(" path: %d\n", path[0]);
            for (j = 1; j < r; j++)
                printf(".%d", path[j]);
        }
        get_vsc_device(dev);
        printf("\n");
    }
}

int vsc_open() {
    int ret = libusb_init(NULL);
    if (ret < 0) {
        printf("libusb failed.\n");
        return -1;
    }

    libusb_device **devs = NULL;
    ret = libusb_get_device_list(NULL, &devs);
    if (ret < 0) {
        printf("no valid usb device.\n");
        return -2;
    }
    print_vsc_devices(devs);
    libusb_free_device_list(devs, 1);

    g_dev_handle = libusb_open_device_with_vid_pid(g_dev_context, \
                                                   DEV_VID, DEV_PID);
    if (g_dev_handle == NULL) {
        printf("cannot open usb device.\n");
        return -3;
    } else {
        printf("usb device opened.\n");
    }

    ret = libusb_claim_interface(g_dev_handle, DEV_INTERFACE);
    if (ret < 0) {
        printf("Cannot claim interface. ret:%d\n", ret);
        return -4;
    }

    printf("vsc_open successfully.\n");
    return 0;
}

int vsc_close() {
    int ret = libusb_release_interface(g_dev_handle, DEV_INTERFACE);
    if (ret != 0) {
        printf("failed to release interface.\n");
        return -1;
    }
    printf("usb interface released!\n");

    libusb_close(g_dev_handle);
    g_dev_handle = NULL;
    libusb_exit(g_dev_context);
    g_dev_context = NULL;
}

int vsc_read(unsigned char *buf, int size, int timeout_ms) {
    int r = 0;
    int actual = 0;
    int ep_no = 1;

    r = libusb_bulk_transfer(g_dev_handle, (ep_no | LIBUSB_ENDPOINT_IN), buf, \
                             size, &actual, timeout_ms);

    if (LIBUSB_ERROR_TIMEOUT == r) {
        printf("read timeout r:%d\n", r);
        r = 1;
        actual = 0;
    } else if (r) {
        actual = 0;
        printf("int_xfer=%d\n", r);
        r = -1;
    }
    if (actual > 0) {
        if (actual == size) {
            r = 0;
        } else {
            r = -2;
            printf("read actual:%d != size:%d [ep: %d]\n",
                   actual, size, ep_no);
        }
    } else {
        r = -3;
    }
    return r;
}

int vsc_write(unsigned char *buf, int size, int timeout_ms) {
    int r = 0;
    int actual = 0;
    int ep_no = 1;

    r = libusb_bulk_transfer(g_dev_handle, (ep_no | LIBUSB_ENDPOINT_OUT), buf, \
            size, &actual, timeout_ms);

    if (LIBUSB_ERROR_TIMEOUT == r) {
        printf("write timeout r:%d\n", r);
        r = 1;
        actual = 0;
    } else if (r) {
        printf("int_xfer=%d\n", r);
        r = -1;
        actual = 0;
    }
    if (actual > 0) {
        if (actual == size) {
            r = 0;
        } else {
            r = -2;
            printf("write actual:%d != size:%d [ep: %d]\n",
                   actual, size, ep_no);
        }
    } else {
        r = -3;
    }
    return r;
}


