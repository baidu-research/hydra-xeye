/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _NET_H
#define _NET_H
#include <string>
#include "platform_pubdef.h"
int NetInit(const XeyePcbType pcb_type,
            const std::string& ip_addr, \
            const std::string& net_mask, \
            const std::string& gate_way);
#endif
