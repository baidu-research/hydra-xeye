/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_DL_PROCESS_H
#define BAIDU_XEYE_DL_PROCESS_H

#include "xeye_message_data_type.h"
#include "xeye_data_type.h"

#include <stdint.h>

#include <vector>
#include <string>

class DlProcess {
public:
    static int preprocess(XeyeConfig &config, XeyeDataItem &data_item);
    static int postprocess(XeyeConfig &config, XeyeDataItem &data_item);
};

#endif // BAIDU_XEYE_DL_PROCESS_H
