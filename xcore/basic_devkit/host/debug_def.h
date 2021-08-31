/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_DEBUG_DEF_H
#define BAIDU_XEYE_DEBUG_DEF_H

#include <stdio.h>

//debug switch
//#define PRINT_DEBUG_INFO

#define UNUSED(x) (void)x

#ifdef PRINT_DEBUG_INFO
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif

#endif //BAIDU_XEYE_DEBUG_DEF_H
