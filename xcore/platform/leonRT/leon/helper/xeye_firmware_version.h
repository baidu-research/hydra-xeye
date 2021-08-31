/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef __XEYE_FIRMWARE_VERSION_H__
#define __XEYE_FIRMWARE_VERSION_H__
#ifdef __cplusplus
extern "C" {
#endif

extern char* get_xeye_version(void);
extern char* get_xeye_compile_by(void);
extern char* get_xeye_compile_host(void);
extern char* get_xeye_compile_date(void);
extern char* get_xeye_compile_time(void);

#ifdef __cplusplus
}
#endif
#endif
