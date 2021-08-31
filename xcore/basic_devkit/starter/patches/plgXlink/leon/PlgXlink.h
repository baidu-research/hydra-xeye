///
/// @file      PlgXlink.h
/// @copyright All code copyright Movidius Ltd 2017, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     PlgXlink header.
///


#ifndef __PLG_XLINK_H__
#define __PLG_XLINK_H__

#ifndef PLGXLINK_MAX_CHAN_NAME
#define PLGXLINK_MAX_CHAN_NAME  (50)
#endif

#include <Flic.h>
#include <Receiver.h>
#include <ImgFrame.h>

class PlgXlink : public ThreadedPlugin
{
public:
    SReceiver<ImgFramePtr> in;

    void  *threadFunc(void *);
    void   Create(uint32_t maxSz, const char* channame);

    uint32_t maxPktSize;
    int      streamId;
    char  channelName[PLGXLINK_MAX_CHAN_NAME];
};

#endif
