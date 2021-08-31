#!/bin/sh
# ----------------------------------------------------
# This script auto-generate xp_sensor_firmware_version.h according to metadata
# Usage: ./mkcompile_h.sh
# ----------------------------------------------------
VERSION_NUM=$(awk -F= '{if($1=="version") print $2}' ../../metadata)
: ${VERSON_NUM:=0.0.1}

MICRO_VERSION_SUFFIX=

COMPILE_BY=`whoami`
COMPILE_HOST=`hostname`
COMPILE_TIME=`date +%T`
COMPILE_DATE=`date +%F`

get_version()
{
    if [ -d .git -o \
         -d ../.git -o \
         -d ../../.git -o \
         -d ../../../.git -o \
         -d ../../../../.git -o \
         -d ../../../../../.git ]; then
        extra="$(git rev-parse --verify --short HEAD)"
        VERSION_EXTRA="-$extra"
        if git diff-index --name-only HEAD |  grep -v "^scripts/package" | read dummy ; then
            VERSION_EXTRA="${VERSION_EXTRA}-dirty!"
        else
            VERSION_EXTRA="${VERSION_EXTRA}-commit"
        fi
    else
        echo -e "\n\n\033[31mERROR: oh no git found\033[37m\n\n"
        exit
    fi
}

get_version

VERSION=$VERSION_NUM$VERSION_EXTRA

cat > leonRT/leon/helper/xeye_firmware_version.c <<_ACEOF

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

#define FIRMWARE_VERSION "V$VERSION"
#define COMPILE_BY "$COMPILE_BY"
#define COMPILE_HOST "$COMPILE_HOST"
#define COMPILE_TIME "$COMPILE_TIME"
#define COMPILE_DATE "$COMPILE_DATE"

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

_ACEOF
