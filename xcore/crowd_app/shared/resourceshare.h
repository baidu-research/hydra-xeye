#ifndef RESOURCESHARE_H
#define RESOURCESHARE_H
//#include <mvTensor_cpp.h>
//#include <mvTensorDebug.h>
#include "pubdef.h"
typedef struct
{
    void* fathomBSS;              /// Area for Fathom to use as intermediate data buffer storage. Must be of a minimal size defined by the network you wish to run.
    int fathomBSS_size;           /// Size of said area.
    int dmaLinkAgent;             /// Direct Memory Access Agent id
    int dataPartitionNo;          /// Number of shave L2 cache data partition (MvTensor use only 1 data partition for all shaves)
    int instrPartitionNo;         /// Number of shave L2 cache partition used for instruction
    int firstShave;               /// First shave allocated for the current FathomRun call
    int lastShave;                /// Last shave allocated for the current FathomRun call
                                  /// (as you can set the number of shaves for each layer, not all layers will run on all the allocated shaves)
    void *dmaTransactions;
    //MvTensorDmaDescriptor *dmaTransactions;
} FathomRunConfig;

typedef struct {
    char modelName[MAX_MODEL_NAME];
    int LayerCount;
    int resizedWidth;
    int resizedHeight;
    int usedFullWidth;
    int usedFullHeight;
    int expectTime;
    int fathomAgent;
    int fathomNum;
    int fathomBssLen;
    int type;
    int graphLen;
    int index;
    char *graph;
    char *modelInBuf;
    int modelOutLen;
    char *modelOutBuf;
    char *fathomBuf;
    procModel procModelDeep;
    void *LayerNet;
    void *mvtp;
    void *resources;
    FathomRunConfig *fathomconfig;
}t_ModelParam;
#endif
