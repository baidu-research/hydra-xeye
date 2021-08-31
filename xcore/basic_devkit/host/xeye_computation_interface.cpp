/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "xeye_computation_interface.h"
#include "xeye_computation.h"
#include "debug_def.h"

int create_xeye_computation_instance(const char* model_file_path, \
                                     const char* config_file_path, \
                                     XEYE_OUTPUT_HANDLER output_handler, \
                                     XEYE_WATCHDOG_HANDLER watchdog_handler, \
                                     IXeyeComputation **xeye_computation) {
    if (NULL == model_file_path || NULL == config_file_path || \
            NULL == output_handler || NULL == watchdog_handler || \
            NULL == xeye_computation) {
        printf("create_xeye_computation_instance invalid parameters.\n");
        return -1;
    }

    *xeye_computation = new XeyeComputation;
    int ret = (*xeye_computation)->init(model_file_path, config_file_path, \
                                        output_handler, watchdog_handler, \
                                        XEYEMODE_COMPUTATION);
    if (0 != ret) {
        printf("xeye init failed. ret:%d\n", ret);
        delete (*xeye_computation);
        *xeye_computation = NULL;
    }
    return ret;
}

int destroy_xeye_computation_instance(IXeyeComputation *xeye_computation) {
    if (NULL != xeye_computation) {
        delete (XeyeComputation *)xeye_computation;
        xeye_computation = NULL;
    }
}
