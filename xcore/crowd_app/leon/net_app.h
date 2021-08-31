/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/


#ifndef _NETAPP_H_
#define _NETAPP_H_

#ifdef  __cplusplus  
extern "C" {  
#endif

typedef int SOCKET;
typedef enum _jpeg_flage_type_
{
    FRAME_INPROCES     = 0,
    FRAME_START        = 1,
    FRAME_END          = 2
}jpeg_flag_type;

typedef enum _common_type_
{
    TYPE_TIME              = 0,
    TYPE_CAMERAID          = 1,
    TYPE_JPEG              = 2,
    TYPE_GOODS             = 3,
    TYPE_FACE              = 4,
    TYPE_HAND              = 5,
    TYPE_BODY              = 6,
    TYPE_JPEGFLAG          = 7,
    TYPE_EVENTID           = 8,
    TYPE_DIFFPOS           = 9,
    TYPE_DEPTHIMG          = 10,
    TYPE_BOTEYEYOUIMG      = 11,
    TYPE_WEIGHTEVENTID     = 12,
    TYPE_WEIGHTSTARTTIME   = 13,
    TYPE_WEIGHTENDTIME     = 14,
    TYPE_GSENSORID         = 15,
    TYPE_GSENSORCELLS      = 16
} net_common_type;

#define CAMERA_ID                  4001
#define TYPE_POS                   24
#define TYPE_SIZE                  4
#define TYPE_FRAMEVER_SIZE         4
#define TYPE_TIME_SIZE             8
#define TYPE_CAMERAID_SIZE         4
#define TYPE_GOODS_SIZE            1414 
#define TYPE_JPEGFLAG_SIZE         4 
#define TYPE_EVENTID_SIZE          8 
#define TYPE_WEIGHTEVENTID_SIZE    24
#define TYPE_WEIGHTSTARTTIME_SIZE  8
#define TYPE_WEIGHTENDTIME_SIZE    8
#define TYPE_GSENSORID_SIZE        4

#define _NET_FRAME_VERSION_ 0

#ifdef  __cplusplus  
}
#endif

#endif // _NETAPP_H_
