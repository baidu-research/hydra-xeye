/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "xeye_interface.h"
#include "xeye.h"
#include "debug_def.h"

#include <stdio.h>

int create_xeye_instance(const char* model_file_path,
                         const char* config_file_path,
                         XEYE_OUTPUT_HANDLER output_handler,
                         XEYE_WATCHDOG_HANDLER watchdog_handler,
                         IXeye **xeye) {
    if (NULL == model_file_path || NULL == config_file_path || \
            NULL == output_handler || NULL == watchdog_handler || \
            NULL == xeye) {
        printf("create_xeye_instance invalid parameters.\n");
        return -1;
    }

    *xeye = new Xeye;
    int ret = (*xeye)->init(model_file_path, config_file_path, \
                            output_handler, watchdog_handler, \
                            XEYEMODE_STANDARD);
    if (0 != ret) {
        printf("xeye init failed. ret:%d\n", ret);
        delete (*xeye);
        *xeye = NULL;
    }
    return ret;
}

void destroy_xeye_instance(IXeye *xeye) {
    if (NULL != xeye) {
        delete (Xeye *)xeye;
    }
}
