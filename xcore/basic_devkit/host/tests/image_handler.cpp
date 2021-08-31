/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "image_handler.h"

#include <stdlib.h>
#include <stdio.h>

int separate_rgb_to_combine_bgr(uint32_t height, uint32_t width, \
                               const uint8_t *rgb_buffer,
                               uint8_t **bgr_buffer) {
    if (0 == height || 0 == width) {
        printf("invalid height:%u width:%u\n", height, width);
        return -1;
    }
    if (NULL == rgb_buffer) {
        printf("error: rgb_buffer is NULL.\n");
        return -1;
    }
    if (NULL == bgr_buffer) {
        printf("error: bgr_buffer is NULL.\n");
        return -1;
    }

    uint32_t channel = 3;
    uint8_t *buf = NULL;
    int ret = 0;
    do {
        buf = (uint8_t *)malloc(height * width * channel);
        if (NULL == buf) {
            printf("failed to malloc %u.\n", height * width * channel);
            ret = -2;
            break;
        }

        const uint8_t *r_buf = rgb_buffer;
        const uint8_t *g_buf = r_buf + height * width;
        const uint8_t *b_buf = g_buf + height * width;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint8_t *bgr_pixel = buf + (y * width + x) * channel;
                const uint8_t *r_pixel = r_buf + (y * width + x);
                const uint8_t *g_pixel = g_buf + (y * width + x);
                const uint8_t *b_pixel = b_buf + (y * width + x);

                bgr_pixel[0] = *b_pixel;
                bgr_pixel[1] = *g_pixel;
                bgr_pixel[2] = *r_pixel;
            }
        }

        *bgr_buffer = buf;
    } while (false);

    if (0 != ret) {
        free(buf);
        buf = NULL;
    }
    return ret;
}
