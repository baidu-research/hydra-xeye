/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "xeye_util.h"

#include <chrono>

uint64_t XeyeUtil::unix_timestamp() {
    std::chrono::time_point<std::chrono::system_clock> p1;
    p1 = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>\
            (p1.time_since_epoch()).count();
}

uint32_t XeyeUtil::align_value(uint32_t value, uint32_t align_value) {
    if (0 == align_value) {
        return value;
    }

    uint32_t remainder = value % align_value;
    if (0 == remainder) {
        return value;
    }
    return (value - remainder + align_value);
}
