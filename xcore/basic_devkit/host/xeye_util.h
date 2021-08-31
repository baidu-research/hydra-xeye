/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_XEYE_UTIL_H
#define BAIDU_XEYE_XEYE_UTIL_H

#include <stdint.h>
#include <opencv/highgui.h>

class XeyeUtil {
public:
    static uint64_t unix_timestamp();
    static uint32_t align_value(uint32_t value, uint32_t align_value);
};

#endif // BAIDU_XEYE_XEYE_UTIL_H
