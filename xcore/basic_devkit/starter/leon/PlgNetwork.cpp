#include "PlgNetwork.h"
#include "mvnciResourceManager.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <FathomMsgInterface.h>
#include <Fp16Convert.h>

#define MVLOG_UNIT_NAME FathomFlicPlg
#include "mvLog.h"
#include "mvMacros.h"
#include "ncCommPrivate.h"

#define GRAPH_VERSION   2

//##############################################################################
#if 0
Network Output Layout:
    |---------------|
    |  512* fp16         |
    |---------------|
    |  1024*fp16        |
    |                        |
    |---------------|
    |  1024*fp16        |
    |                        |
    |---------------|
    |  1024*fp16        |
    |                        |
    |---------------|
#endif

void PlgNetwork::Create(const PlgNetworkCfg* cfg)
{
    assert(NULL != cfg);

    memcpy(&local_cfg, cfg, sizeof(PlgNetworkCfg));

    inF.Create(1, QueueCmdOvr<ImgFramePtr>::Inst());

    Add(&inF , ".inF");
    Add(&output, ".output");
    Add(&resultInput, ".resultInput");

    networkStatus = MvNCILoadNetwork(&det_network, local_cfg.det_parm.blobFile, -1);
    if (networkStatus != MVNCI_SUCCESS) {
        printf("Failed to load det network\n");
        assert(0);
    }

    updateBlobSpecificInfo(det_network, &det_nstages, &det_inputCount, &det_outputCount, det_inputDescriptors, det_outputDescriptors, det_blob_version);

    //Application can override these values and/or provide external buffers after calling this method
    memoryResources = { { nullptr, 0 }, { nullptr, 0 }, { nullptr, 0 } };
    hwConfig.shaveL2DataPartition = local_cfg.hw.data_partID;    //MvTensor uses the same partion with pre-proc's
    hwConfig.shaveL2InstructionPartition = local_cfg.hw.inst_partID;
}

void PlgNetwork::Delete(){
    MvNCIErrorCode status;
    status = MvNCIReleaseNetwork(det_network);
    assert(status == MVNCI_SUCCESS);
}

unsigned int PlgNetwork::getTotalSize(fathomTensorDescriptor_t* desc) {
    unsigned int maxStride;
    unsigned int maxDim;

    if (desc->widthStride == desc->heightStride &&
        desc->widthStride == desc->channelsStride) {
        maxDim = MAX(desc->w, desc->h);
        maxDim = MAX(maxDim, desc->c);
        maxStride = desc->widthStride;
    } else if (desc->widthStride >= desc->heightStride &&
               desc->widthStride >= desc->channelsStride) {
        maxStride = desc->widthStride;
        maxDim = desc->w;
        if (desc->widthStride == desc->heightStride)
            maxDim = MAX(desc->w, desc->h);
        else if (desc->widthStride == desc->channelsStride)
            maxDim = MAX(desc->w, desc->c);
    } else if (desc->heightStride >= desc->widthStride &&
               desc->heightStride >= desc->channelsStride) {
        maxStride = desc->heightStride;
        maxDim = desc->h;
        if (desc->heightStride == desc->widthStride)
            maxDim = MAX(desc->h, desc->w);
        else if (desc->heightStride == desc->channelsStride)
            maxDim = MAX(desc->h, desc->c);
    } else {
        maxStride = desc->channelsStride;
        maxDim = desc->c;
        if (desc->channelsStride == desc->widthStride)
            maxDim = MAX(desc->c, desc->w);
        else if (desc->channelsStride == desc->heightStride)
            maxDim = MAX(desc->c, desc->h);
    }
    return desc->n * maxStride * maxDim;
}

void PlgNetwork::updateBlobSpecificInfo(MvNCINetworkHandle network, unsigned int *nstages,
    unsigned int *inputCount, unsigned int *outputCount, fathomTensorDescriptor_t* inputDescriptors,
    fathomTensorDescriptor_t* outputDescriptors, uint32_t *blob_version){
    MvNCIErrorCode status = MvNCIGetStageCount(network, nstages);
    assert(status == MVNCI_SUCCESS);
    status = MvNCIGetInputCount(network, inputCount);
    assert(status == MVNCI_SUCCESS);
    status = MvNCIGetOutputCount(network, outputCount);
    assert(status == MVNCI_SUCCESS);
    unsigned int id;
    assert((*inputCount) <= MAX_INPUTS);
    assert((*outputCount) <= MAX_OUTPUTS);
    for(id = 0; id < (*inputCount); id++){
        MvNCITensorShape shape;
        status = MvNCIGetInputShape(network, id, &shape);
        assert(status == MVNCI_SUCCESS);
        inputDescriptors[id].c = shape.channels;
        inputDescriptors[id].h = shape.height;
        inputDescriptors[id].w = shape.width;
        inputDescriptors[id].n = 1; //TODO: this is batch size
        inputDescriptors[id].channelsStride = shape.channelsStride;
        inputDescriptors[id].widthStride = shape.widthStride;
        inputDescriptors[id].heightStride = shape.heightStride;
        inputDescriptors[id].totalSize = getTotalSize(&inputDescriptors[id]);
    }

    for(id = 0; id < (*outputCount); id++){
        MvNCITensorShape shape;
        status = MvNCIGetOutputShape(network, id, &shape);
        assert(status == MVNCI_SUCCESS);
        outputDescriptors[id].c = shape.channels;
        outputDescriptors[id].h = shape.height;
        outputDescriptors[id].w = shape.width;
        outputDescriptors[id].n = 1; //TODO: this is batch size
        outputDescriptors[id].channelsStride = shape.channelsStride;
        outputDescriptors[id].widthStride = shape.widthStride;
        outputDescriptors[id].heightStride = shape.heightStride;
        outputDescriptors[id].totalSize = getTotalSize(&outputDescriptors[id]);
    }
    MvNCIVersion mvBlobVer;
    status = MvNCIGetBlobVersion(network, &mvBlobVer);
    assert(status == MVNCI_SUCCESS);
    blob_version[0] = mvBlobVer.major;
    blob_version[1] = mvBlobVer.minor;
}

int  PlgNetwork::crop_valid(float x1, float y1, float x2, float y2)
{
    if((x1<0) || (x1>1))
        return 0;
    if((y1<0) || (y1>1))
        return 0;
    if((x2<0) || (x2>1))
        return 0;
    if((y2<0) || (y2>1))
        return 0;
    if((x1>=x2) || (y1>=y2))
        return 0;

    return 1;
}

void PlgNetwork::DetectionPreProc(ImgFrame *frame)
{
    MovidiusImage inParams, outParams;
    inParams.nWidth  = frame->fb.spec.width;
    inParams.nHeight = frame->fb.spec.height;
    inParams.nFormat = local_cfg.det_parm.inFormat;
    inParams.ws      = 0;  //Crop start X
    inParams.hs      = 0;  //Crop start Y
    inParams.we     = frame->fb.spec.width;  //Crop end X
    inParams.he      = frame->fb.spec.height;  //Crop end Y
    inParams.pData   = (fp16 *)frame->base;

    outParams.nWidth     = local_cfg.det_parm.width;
    //outParams.nWidthPad  = (outParams.nWidth%8)==0 ? outParams.nWidth : outParams.nWidth+8-(outParams.nWidth%8);
    outParams.nHeight    = local_cfg.det_parm.height;
    outParams.nFormat    = local_cfg.det_parm.outFormat;
    outParams.mean_value[0] = local_cfg.det_parm.mean_value[0];
    outParams.mean_value[1] = local_cfg.det_parm.mean_value[1];
    outParams.mean_value[2] = local_cfg.det_parm.mean_value[2];
    outParams.std_value  = local_cfg.det_parm.std_value;
    outParams.pData      = (fp16 *)local_cfg.ddr_mem.output;

    NNPreProcConfig	resource_cfg;
    resource_cfg.firstShave = local_cfg.hw.first_shave;
    resource_cfg.lastShave = local_cfg.hw.last_shave;
    resource_cfg.data_partID = local_cfg.hw.data_partID;
    resource_cfg.inst_partID = local_cfg.hw.inst_partID;
    resource_cfg.tmpbuf = local_cfg.ddr_mem.tmpbuf;

    NNPreProcess(&inParams, &outParams, &resource_cfg);

}

void PlgNetwork::ClassificationPreProc(ImgFrame *frame, float x1, float y1, float x2, float y2)
{
    MovidiusImage inParams, outParams;
    inParams.nWidth  = frame->fb.spec.width;
    inParams.nHeight = frame->fb.spec.height;
    inParams.nFormat = local_cfg.cls_parm.inFormat;
    inParams.ws      = x1 * frame->fb.spec.width;;  //Crop start X
    inParams.hs      = y1 * frame->fb.spec.height;  //Crop start Y
    inParams.we      = x2 * frame->fb.spec.width;  //Crop end X
    inParams.he      = y2 * frame->fb.spec.height;  //Crop end Y
    inParams.pData   = (fp16 *)frame->base;

    outParams.nWidth     = local_cfg.cls_parm.width;
    //outParams.nWidthPad  = (outParams.nWidth%8)==0 ? outParams.nWidth : outParams.nWidth+8-(outParams.nWidth%8);
    outParams.nHeight    = local_cfg.cls_parm.height;
    outParams.nFormat    = local_cfg.cls_parm.outFormat;
    outParams.mean_value[0] = local_cfg.cls_parm.mean_value[0];
    outParams.mean_value[1] = local_cfg.cls_parm.mean_value[1];
    outParams.mean_value[2] = local_cfg.cls_parm.mean_value[2];
    outParams.std_value  = local_cfg.cls_parm.std_value;
    outParams.pData      = (fp16 *)local_cfg.ddr_mem.output;

    NNPreProcConfig	resource_cfg;
    resource_cfg.firstShave = local_cfg.hw.first_shave;
    resource_cfg.lastShave = local_cfg.hw.last_shave;
    resource_cfg.data_partID = local_cfg.hw.data_partID;
    resource_cfg.inst_partID = local_cfg.hw.inst_partID;
    resource_cfg.tmpbuf = local_cfg.ddr_mem.tmpbuf;

    NNPreProcess(&inParams, &outParams, &resource_cfg);
}

void PlgNetwork::run_detection(fp16 *output)
{
    while(is_config_set == 0)
            usleep(1); // TODO: use condition variable here. This is not as bad as it seems, as the config will be set already when the first tensor arrives. This is just a safety measure
    if (local_cfg.det_parm.blobFile== nullptr)
    {
        printf("Can't find blob\n");
        assert(0);
    }
    void * fathom_input = (void *)local_cfg.ddr_mem.output;  //Set pre-proc outputbuffer to fathom input

    struct timespec ts, ts2;
    clock_gettime(CLOCK_REALTIME, &ts);

    MvNCIErrorCode status = MVNCI_SUCCESS;

    state = GRAPH_RUNNING;

    status = MvNCIRun(det_executor, &memoryResources,
                      &fathom_input, (void* const*)&output,
                      nullptr, nullptr);

    if (status != MVNCI_SUCCESS)
    {
        printf("MvNCIRun failed, %d\n", status);
        assert(0);
    }

    clock_gettime(CLOCK_REALTIME, &ts2);

    int64_t nsec;
    nsec = ts2.tv_nsec - ts.tv_nsec;
    if (nsec < 0){
        nsec += 1*1000*1000* 1000; // 1 second
    }

//    #define TIME_OUTER
    #ifdef TIME_OUTER
        printf("[%s]:Finished Fathom in %f ms\n", "MobileNetSSD", nsec/1000000.0);
    #endif
}

void PlgNetwork::run_classification(fp16 *output)
{
    while(is_config_set == 0)
            usleep(1); // TODO: use condition variable here. This is not as bad as it seems, as the config will be set already when the first tensor arrives. This is just a safety measure
    if (local_cfg.cls_parm.blobFile== nullptr)
    {
        printf("Can't find blob\n");
        assert(0);
    }
    void * fathom_input = (void *)local_cfg.ddr_mem.output;  //Set pre-proc outputbuffer to fathom input

    struct timespec ts, ts2;
    clock_gettime(CLOCK_REALTIME, &ts);

    MvNCIErrorCode status = MVNCI_SUCCESS;

    state = GRAPH_RUNNING;

    status = MvNCIRun(cls_executor, &memoryResources,
                      &fathom_input, (void* const*)&output,
                      nullptr, nullptr);

    if (status != MVNCI_SUCCESS)
    {
        printf("MvNCIRun failed, %d\n", status);
        assert(0);
    }

    clock_gettime(CLOCK_REALTIME, &ts2);

    int64_t nsec;
    nsec = ts2.tv_nsec - ts.tv_nsec;
    if (nsec < 0){
        nsec += 1*1000*1000* 1000; // 1 second
    }

//    #define TIME_OUTER
    #ifdef TIME_OUTER
        printf("[%s]:Finished Fathom in %f ms\n", "MobileNet", nsec/1000000.0);
    #endif
}


void *PlgNetwork::threadFunc(void *arg)
{
  rtems_object_set_name(rtems_task_self(), "PlgNetwork_threadFunc");
  UNUSED(arg);
  int err = 0;

  MvNCIErrorCode status;
  status = MvNCIPrepareNetwork(&det_executor, det_network,
                                 &processingResources,
                                 &memoryResources,
                                 &hwConfig);
  assert(status == MVNCI_SUCCESS);

  while(Alive())
  {
    ImgFramePtr inFrm;
    ImgFramePtr outTsr;

    err = inF.Receive(&inFrm);
    err += resultInput.Receive(&outTsr);
    if (!err)
    {
        ImgFrame *curF = inFrm.ptr;
        DetectionPreProc(curF);
        fp16 *det_output = (fp16 *)outTsr.ptr->base;
        run_detection(det_output);
        int res = PostProcessing(det_output);
        if(!res)
        {
        	continue;
        }
        outTsr.ptr->fb.spec.width = outTsr.ptr->size;
        outTsr.ptr->fb.spec.type = BITSTREAM;
        output.Send(&outTsr);
    }
  }
  return NULL;
}

