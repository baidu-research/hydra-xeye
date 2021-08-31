/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "watchdog_handler.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/reboot.h>

void watchdog_handler() {
    printf("#### enter watchdog_handler ####\n");
    sync();
    int ret = -1; //reboot(RB_AUTOBOOT);
    if (0 == ret) {
        printf("reboot successfully.\n");
    } else {
        printf("failed to reboot. ret:%d", ret);
    }
    abort();
}
