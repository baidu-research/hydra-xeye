/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_IMAGE_HANDLER_H
#define BAIDU_XEYE_IMAGE_HANDLER_H

#include <stdint.h>
#include <opencv/highgui.h>

int separate_rgb_to_combine_bgr(uint32_t height, uint32_t width, \
                               const uint8_t *rgb_buffer,
                               uint8_t **bgr_buffer);

#endif // BAIDU_XEYE_IMAGE_HANDLER_H
