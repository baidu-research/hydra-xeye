#ifndef __PLG_NETWORK_H__
#define __PLG_NETWORK_H__

#include <Flic.h>
#include <Sender.h>
#include <Receiver.h>
#include <BehQueueOvr.h>
#include "ImgFrame.h"
#include "FathomMsgInterface.h"
#include "NNPreProcess.h"

#include <DrvSvuDefines.h>
#include <Pool.h>
#include <PoolObj.h>
#include <MemAllocator.h>
#include <mvnci.h>

#ifndef MAX_OBJ
#define MAX_OBJ 10
#endif
#define MAX_INPUTS 4
#define MAX_OUTPUTS 4
#define MAX_NAME_SIZE 20

#define DEBUG_BUFFER_SIZE 120

typedef struct PreProcMemory
{
    u8 *tmpbuf;
    u8 *output;
} PreProcMemory;

typedef struct NNHwCfg
{
    int first_shave;
    int last_shave;
    int data_partID;
    int inst_partID;
} NNHwCfg;

typedef struct NetworkParam
{
  void *blobFile;
  int dim;
  int width;
  int height;
  IMAGE_FORMAT inFormat;
  IMAGE_FORMAT outFormat;
  float mean_value[3];
  float std_value;
} NetworkParam;

typedef struct PlgNetworkCfg
{
    NetworkParam  det_parm;
    NetworkParam  cls_parm;
    NNHwCfg         hw;
    PreProcMemory ddr_mem;
} PlgNetworkCfg;
typedef struct SunnyImg
{
	u8* Img;
	frameType      type;
	unsigned int   height;    // width in pixels
	unsigned int   width;     // width in pixels
}SunnyImg;
//############################################################
class PlgNetwork : public ThreadedPlugin
{
 public:

 //FLIC IOs
    SReceiver<ImgFramePtr>  inF; //input  Image and crop info
    MReceiver<ImgFramePtr> resultInput;
    MSender<ImgFramePtr> output;

    void *threadFunc(void *);
    void Create(const PlgNetworkCfg* cfg);
    int crop_valid(float x1, float y1, float x2, float y2);

    void DetectionPreProc(ImgFrame *frame);
    void ClassificationPreProc(ImgFrame *frame, float x1, float y1, float x2, float y2);
    int PostProcessing(fp16* output);
    void run_detection(fp16 *output);
    void run_classification(fp16 *output);

    // functions below are implemented in SunnyLib.a
    void NetCreate(const PlgNetworkCfg* cfg);
    void NetPreProc(SunnyImg *frame);
    void ComputationModeInit(u8* InBlob);
    void ComputationModeRun(u8* NNout);

//////////////////////////////////////////////////////////

    // this plugin is exporting these. TODO: getters, but no setters
    unsigned int det_nstages;
    unsigned int det_inputCount;
    unsigned int det_outputCount;

    unsigned int cls_nstages;
    unsigned int cls_inputCount;
    unsigned int cls_outputCount;

    MvNCIErrorCode networkStatus;
    fathomTensorDescriptor_t det_inputDescriptors[MAX_INPUTS];
    fathomTensorDescriptor_t det_outputDescriptors[MAX_OUTPUTS];
    fathomTensorDescriptor_t cls_inputDescriptors[MAX_INPUTS];
    fathomTensorDescriptor_t cls_outputDescriptors[MAX_OUTPUTS];

    uint32_t det_blob_version[2];
    uint32_t cls_blob_version[2];


    MvNCIProcessingResources processingResources;
    MvNCIHWConfig hwConfig = {
            1, //data partition
            0 // instr partition
    };

    MvNCIMemoryResources memoryResources;
    //TODO: reduce the public variables here
    int is_config_set = 0;

    void Create(void *blobFile, unsigned int blobLength, char *name);
    void Delete();
  private:
    unsigned int getTotalSize(fathomTensorDescriptor_t* desc);

    int    state;

    MvNCINetworkHandle det_network = nullptr;
    MvNCINetworkHandle cls_network = nullptr;
    MvNCIExecutorHandle det_executor;
    MvNCIExecutorHandle cls_executor;

    void updateBlobSpecificInfo(MvNCINetworkHandle network, unsigned int *nstages,
    unsigned int *inputCount, unsigned int *outputCount, fathomTensorDescriptor_t* inputDescriptors,
    fathomTensorDescriptor_t* outputDescriptors, uint32_t *blob_version);

 private:
    PlgNetworkCfg local_cfg;
};

#endif
