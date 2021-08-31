/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include <stdio.h>

#define FIRMWARE_VERSION "V--commit"
#define COMPILE_BY "haofeng"
#define COMPILE_HOST "system"
#define COMPILE_TIME "14:50:12"
#define COMPILE_DATE "2019-08-12"

char* get_xeye_version(void) {
    static char buf[100];
    sprintf(buf, "%s", FIRMWARE_VERSION);
    return &buf[0];
}

char* get_xeye_compile_by(void) {
    static char buf[100];
    sprintf(buf, "%s", COMPILE_BY);
    return &buf[0];
}

char* get_xeye_compile_host(void) {
    static char buf[100];
    sprintf(buf, "%s", COMPILE_HOST);
    return &buf[0];
}

char* get_xeye_compile_date(void) {
    static char buf[100];
    sprintf(buf, "%s", COMPILE_DATE);
    return &buf[0];
}

char* get_xeye_compile_time(void) {
    static char buf[100];
    sprintf(buf, "%s", COMPILE_TIME);
    return &buf[0];
}

