///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Fathom Flic messages
///
#ifndef _FATHOM_FLIC_MSG_I_H_
#define _FATHOM_FLIC_MSG_I_H_



#include <Flic.h>
#include <Sender.h>
#include <Receiver.h>
#include <BehQueueOvr.h>
#include <Pool.h>
#include <PoolObj.h>
#include <MemAllocator.h>
#include <XLink.h>

struct fathomTensorDescriptor_t{
    unsigned int n;
    unsigned int c;
    unsigned int w;
    unsigned int h;
    unsigned int totalSize;
    unsigned int widthStride;
    unsigned int heightStride;
    unsigned int channelsStride;
};


struct TensorMsg : public PoBuf{
public:
    uint8_t* data;
    fathomTensorDescriptor_t desc;
    int streamId;
    TensorMsg(){
        streamId = INVALID_STREAM_ID;
        data = NULL;
        size = 0;
    }
};

typedef PoPtr<TensorMsg> TensorMsgPtr;


typedef struct pluginIO_t{
    MSender<TensorMsgPtr>* sender;
    SReceiver<TensorMsgPtr>* receiver;
}pluginIO_t;

typedef struct fifoIO_t{
    MSender<TensorMsgPtr>* primarySender; // connects to the fifo input FLIC port which will be consumed by XLink
    SReceiver<TensorMsgPtr>* primaryReceiver; // connects to the fifo output FLIC port which is produced from an XLink channel
}fifoIO_t;

struct graphTriggerMsg : public PoBuf{
    fifoIO_t bufferIo;
    pluginIO_t graphIo;
};


#endif
