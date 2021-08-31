/*
 * fathom.c
 *
 *  Created on: Jun 24, 2016
 *      Author: ian-movidius
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef MA2480
#include <DrvShaveL2c.h>
#else
#include <OsDrvShaveL2Cache.h>
#endif
#include <UnitTestApi.h>
#include <VcsHooksApi.h>
#include <DrvLeonL2C.h>
#include <Fp16Convert.h>
#include <mvTensor_cpp.h>
#include <mvTensorDebug.h>
//#include "definecpp.h"
#include "resourceshare.h"
#include "define.h"
#include "fathomRun.h"
#include "pubdef.h"

#define BLOB_STRING_LENGTH 100

#define MAX_STAGE 100 
unsigned char x_char;
unsigned int x_uint;
unsigned short x_short;

//#define DPRINTF(...) printf(__VA_ARGS__)
#define DPRINTF(...)
void initModelType(char *modelName, t_ModelParam * modelParam)
{
    modelParam->procModelDeep = NULL;
    return;
}

void generateMvTensorCalls(unsigned char * blobNetworkStart, unsigned int total_stages,
        unsigned int offset_to_weights, t_MvTensorParam * fathomNet,
        t_MvTensorMyriadResources* myriadResourcePool,
        t_MvMatMulMyriadResources* matmulResourcesPool,
        t_mvTensorGenData* inputStruct, t_mvTensorGenData* weightsStruct,
        t_mvTensorGenData* biasStruct, t_mvTensorGenData* outputStruct,
        t_MvTensorOp* preOpStruct, t_MvTensorOp* opStruct , t_MvTensorOp* postOpStruct,
        t_MvTensorDebugInfo* dbgInfoArray, FathomRunConfig* fathomResources, short first_shave,
        short last_shave, void* inputTensor, void* outputTensor,
        unsigned int cache_memory_size, unsigned int scratch_memory_size,
        char *cache_memory_ptr, char *scratch_memory_ptr, unsigned int *offset_tracker,int *piMaxOffset);

unsigned char* readBlobString(unsigned char* location, int size, unsigned int* offset);

template <typename T> T readBlob(unsigned char* location, T* data_type, unsigned int* offset);


t_MvMatMulMyriadResources *configureMatMulResources(unsigned int cache_memory_size, unsigned int scratch_memory_size,
        char *cache_memory_ptr, char *scratch_memory_ptr,
        t_MvMatMulMyriadResources* matmulResources);


mv::tensor::Processor *mvtps[MAX_GRAPH] = {0};

int FathomInitModel(t_ModelParam * modelParam, unsigned char * blob, int blob_size, void * inputTensor, void * outputTensor,
        FathomRunConfig * fathomResources,  u8 * debugBuffer,
        unsigned int cache_memory_size, unsigned int scratch_memory_size,
        char *cache_memory_ptr, char *scratch_memory_ptr, int print_times, int debug){

    unsigned int offset_tracker = 0;
    int stage_count = 0;

    (void)print_times;
    offset_tracker = BLOB_FILE_SIZE_OFFSET + sizeof(blob_size);

    //Read Header items
    unsigned int version = readBlob(&blob[offset_tracker], &x_uint, &offset_tracker);
    unsigned char * network_name = readBlobString(&blob[offset_tracker], BLOB_STRING_LENGTH, &offset_tracker);
    unsigned char * report_dir = readBlobString(&blob[offset_tracker], BLOB_STRING_LENGTH, &offset_tracker);

    stage_count = readBlob(&blob[offset_tracker], &x_uint, &offset_tracker);
    unsigned int offset_to_weights = readBlob(&blob[offset_tracker], &x_uint, &offset_tracker);

    if(debug) {
        DPRINTF("BLOB LOCATION %x\n",blob);
        DPRINTF("FATHOM VERSION HEX %x\n",version);
        DPRINTF("Fathom v%u: '%s' .\n",version, network_name);
        DPRINTF("Outputting to:\n %s\n", report_dir);
    }

    unsigned short first_shave_blob = (unsigned short)readBlob(&blob[offset_tracker], &x_short, &offset_tracker);
    unsigned short last_shave_blob = (unsigned short)readBlob(&blob[offset_tracker], &x_short, &offset_tracker);
    if (modelParam->fathomAgent == 0)
    {
        first_shave_blob = 4;//(unsigned short)readBlob(&blob[offset_tracker], &x_short, &offset_tracker);
        last_shave_blob = 7;//(unsigned short)readBlob(&blob[offset_tracker], &x_short, &offset_tracker);
    }
    else
    {
        
        first_shave_blob = 8;//(unsigned short)readBlob(&blob[offset_tracker], &x_short, &offset_tracker);
        last_shave_blob = 11;//(unsigned short)readBlob(&blob[offset_tracker], &x_short, &offset_tracker);
    }


//    unsigned short first_shave_blob = (unsigned short)readBlob(&blob[offset_tracker], &x_short, &offset_tracker);
//    unsigned short last_shave_blob = (unsigned short)readBlob(&blob[offset_tracker], &x_short, &offset_tracker);
    int blob_requested_shaves = last_shave_blob - first_shave_blob + 1;

    int number_of_allocated_shaves = fathomResources->lastShave - fathomResources->firstShave + 1;

    ErrorManager::Assert(blob_requested_shaves <= number_of_allocated_shaves, "Number of allocated shaves smaller then the number of requested -inside blob- shaves");

    int first_shave = first_shave_blob + fathomResources->firstShave;
    int last_shave = last_shave_blob + fathomResources->firstShave;

    if (1){
        printf("Shaves: %d:%d\n", first_shave, last_shave);
    }

    readBlobString(&blob[offset_tracker], 12, &offset_tracker); // Myriad Resources from Blob - Unused right now.

    t_MvTensorDebugInfo * dbgInfo = (t_MvTensorDebugInfo *)malloc(stage_count * sizeof(t_MvTensorDebugInfo));
    assert(dbgInfo != nullptr);

    if(debugBuffer){
        for(u32 i = 0; i != stage_count; i++){
            dbgInfo[i].debugMsg = (char *)debugBuffer;
            dbgInfo[0].ms = 0;
        }
        ErrorManager::Init((char *)debugBuffer);
    }else{
        for(u32 i = 0; i != stage_count; i++){
            dbgInfo[i].debugMsg = NULL;
            dbgInfo[0].ms = 0;
        }
        ErrorManager::Init(NULL);
    }

    t_MvTensorParam *fathomNet = (t_MvTensorParam *)malloc(stage_count * sizeof(t_MvTensorParam));
    ErrorManager::Assert(fathomNet != nullptr, "malloc failed in FathomRun");
    t_mvTensorGenData *inputStruct = (t_mvTensorGenData *)malloc(stage_count * sizeof(t_mvTensorGenData));
    ErrorManager::Assert(inputStruct != nullptr, "malloc failed in FathomRun");
    t_mvTensorGenData *outputStruct = (t_mvTensorGenData *)malloc(stage_count * sizeof(t_mvTensorGenData));
    ErrorManager::Assert(outputStruct != nullptr, "malloc failed in FathomRun");
    t_mvTensorGenData *weightsStruct = (t_mvTensorGenData *)malloc(stage_count * sizeof(t_mvTensorGenData));
    ErrorManager::Assert(weightsStruct != nullptr, "malloc failed in FathomRun");
    t_mvTensorGenData *biasStruct = (t_mvTensorGenData *)malloc(stage_count * sizeof(t_mvTensorGenData));
    ErrorManager::Assert(biasStruct != nullptr, "malloc failed in FathomRun");
    t_MvTensorOp *preOpStruct = (t_MvTensorOp *)malloc(stage_count * sizeof(t_MvTensorOp));
    ErrorManager::Assert(preOpStruct != nullptr, "malloc failed in FathomRun");
    t_MvTensorOp *opStruct = (t_MvTensorOp *)malloc(stage_count * sizeof(t_MvTensorOp));
    ErrorManager::Assert(opStruct != nullptr, "malloc failed in FathomRun");
    t_MvTensorOp *postOpStruct = (t_MvTensorOp *)malloc(stage_count * sizeof(t_MvTensorOp));
    ErrorManager::Assert(postOpStruct != nullptr, "malloc failed in FathomRun");
    t_MvTensorMyriadResources * myriadResourcePool = (t_MvTensorMyriadResources *)malloc(stage_count * sizeof(t_MvTensorMyriadResources));
    ErrorManager::Assert(myriadResourcePool != nullptr, "malloc failed in FathomRun");
    t_MvMatMulMyriadResources *matmulResourcesPool = (t_MvMatMulMyriadResources *)malloc(stage_count * sizeof(t_MvMatMulMyriadResources));
    ErrorManager::Assert(matmulResourcesPool != nullptr, "malloc failed in FathomRun");
    generateMvTensorCalls(blob, stage_count, offset_to_weights, fathomNet, myriadResourcePool, matmulResourcesPool,
            inputStruct, weightsStruct, biasStruct, outputStruct,
            preOpStruct, opStruct, postOpStruct, dbgInfo, fathomResources, first_shave, last_shave, inputTensor, outputTensor,
            cache_memory_size, scratch_memory_size, cache_memory_ptr, scratch_memory_ptr, &offset_tracker,&modelParam->fathomBssLen);
    printf("fathomNet %x\n",fathomNet);
    t_MvTensorMyriadResources commonMyriadResourcesForAllLayers =
    {{{
firstShave : first_shave,
             lastShave : last_shave,
             dmaLinkAgent : fathomResources->dmaLinkAgent,
             dataPartitionNo : fathomResources->dataPartitionNo,
             instrPartitionNo : fathomResources->instrPartitionNo,
             dmaTransactions : (MvTensorDmaDescriptor*)fathomResources->dmaTransactions,
      }}};

    t_MvMatMulMyriadResources commonMatMulResourcesForAllLayers =
    {
cache_memory_size : cache_memory_size,
                    scratch_memory_size : scratch_memory_size,
                    cache_memory_ptr : cache_memory_ptr,
                    scratch_memory_ptr : scratch_memory_ptr,
    };

    t_MvTensorDebugInfo debugInfo =
    {
ms : 0,
     debugMsg : reinterpret_cast<char *>(debugBuffer),
    };

        // mv::tensor::Processor *mvtp = NULL;
    if(mvtps[modelParam->fathomAgent] == NULL){
        printf("init mvtp\n");
        //mvtps[modelParam->fathomAgent] = new mv::tensor::Processor(commonMyriadResourcesForAllLayers, &commonMatMulResourcesForAllLayers, &debugInfo);
    }

    strcpy(modelParam->modelName, (char *)network_name);
    modelParam->LayerCount = stage_count;
    modelParam->LayerNet = (void *)fathomNet;
    modelParam->mvtp = (void *)mvtps[modelParam->fathomAgent];
    modelParam->resizedWidth = fathomNet[0].input->dimX;
    modelParam->resizedHeight = fathomNet[0].input->dimY;

    modelParam->modelOutLen = fathomNet[stage_count-1].output->dimX * fathomNet[stage_count-1].output->dimY * fathomNet[stage_count-1].output->dimZ;
    modelParam->modelOutBuf = (char *)fathomNet[stage_count-1].output->data;
    initModelType((char *)network_name,modelParam);
    printf("init fathom finish.\n");
    return 0;
}

void fathomrun_every (t_ModelParam * modelParam)
{
    int first_shave = 0;
    int last_shave = 0;
    double callDuration = 0;
    int stage_count = modelParam->LayerCount;
    t_MvTensorParam *fathomNet = (t_MvTensorParam*)modelParam->LayerNet;
    u8 * debugBuffer = NULL;
  
    t_MvTensorMyriadResources commonMyriadResourcesForAllLayers;
  
    first_shave = fathomNet[0].myriadResources->firstShave;
    last_shave = fathomNet[0].myriadResources->lastShave;
    commonMyriadResourcesForAllLayers.firstShave = fathomNet[0].myriadResources->firstShave;
    commonMyriadResourcesForAllLayers.lastShave = fathomNet[0].myriadResources->lastShave;
    commonMyriadResourcesForAllLayers.dmaLinkAgent = fathomNet[0].myriadResources->dmaLinkAgent;
    commonMyriadResourcesForAllLayers.dataPartitionNo = fathomNet[0].myriadResources->dataPartitionNo;
    commonMyriadResourcesForAllLayers.instrPartitionNo = fathomNet[0].myriadResources->instrPartitionNo;
    commonMyriadResourcesForAllLayers.dmaTransactions = fathomNet[0].myriadResources->dmaTransactions;
  
    t_MvMatMulMyriadResources commonMatMulResourcesForAllLayers;
    commonMatMulResourcesForAllLayers.cache_memory_size = fathomNet[0].matmulResources->cache_memory_size;
    commonMatMulResourcesForAllLayers.scratch_memory_size = fathomNet[0].matmulResources->scratch_memory_size;
    commonMatMulResourcesForAllLayers.cache_memory_ptr = fathomNet[0].matmulResources->cache_memory_ptr;
    commonMatMulResourcesForAllLayers.scratch_memory_ptr = fathomNet[0].matmulResources->scratch_memory_ptr;
  
    t_MvTensorDebugInfo debugInfo =
    {
      ms : 0,
      debugMsg : reinterpret_cast<char *>(debugBuffer),
    };
  
    mv::tensor::Processor mvtp(commonMyriadResourcesForAllLayers, &commonMatMulResourcesForAllLayers, &debugInfo);
    
    for(unsigned int stage = 0; stage < stage_count; stage++)
    {
        mvtp.run(fathomNet[stage]);
  
        DPRINTF("MvTensor Call#: %d/%d OpType: %x\n", stage + 1, stage_count, fathomNet[stage].op->type);
        DPRINTF("MvTensor done in: %f ms\n", fathomNet[stage].debugInfo->ms);
  
        callDuration += fathomNet[stage].debugInfo->ms;
    }
    if (stage_count > 30) {  //Fathomrun run layer 29,will occur different value.
        int ox = fathomNet[29].output->dimX;
        int oy = fathomNet[29].output->dimY;
        int oz = fathomNet[29].output->dimZ;
        memset(fathomNet[29].output->data, 0, ox*oy*oz*2);
    }
  
    OsDrvShaveL2CachePartitionFlush(fathomNet[stage_count-1].myriadResources->dataPartitionNo, PERFORM_INVALIDATION);
    OsDrvShaveL2CachePartitionInvalidate(fathomNet[stage_count-1].myriadResources->instrPartitionNo);
    
    unsigned int outDataSize = fathomNet[stage_count-1].output->dimX *
                               fathomNet[stage_count-1].output->dimY *
                               fathomNet[stage_count-1].output->dimZ * sizeof(fp16);
  
    rtems_cache_invalidate_multiple_data_lines((void *)((u32)fathomNet[stage_count-1].output->data), outDataSize);
    //printf("MvTensor total time: %f ms\n", callDuration);
}

void fathomRun(t_ModelParam * modelParam)
{
    double callDuration = 0;
    int stage_count = modelParam->LayerCount;

    mv::tensor::Processor *mvtp = (mv::tensor::Processor*)modelParam->mvtp;
    t_MvTensorParam *fathomNet = (t_MvTensorParam*)modelParam->LayerNet;
    for (int stage = 0; stage < stage_count; stage++) {
        mvtp->run(fathomNet[stage]);
        callDuration += fathomNet[stage].debugInfo->ms;
    }
    //DrvShaveL2CachePartitionFlush(0);
    //DrvShaveL2CachePartitionInvalidate(1);
    //DrvLL2CFlushOpOnAddrRange(LL2C_OPERATION_INVALIDATE, 0,
    //        (u32)fathomNet[stage_count-1].output->data,
    //        (u32)fathomNet[stage_count-1].output->data +
    //        (fathomNet[stage_count-1].output->dimX *
    //         fathomNet[stage_count-1].output->dimY *
    //         fathomNet[stage_count-1].output->dimZ) * sizeof(fp16));
}



void generateMvTensorCalls(unsigned char * blobNetworkStart, unsigned int total_stages, unsigned int offset_to_weights, t_MvTensorParam * fathomNet,
        t_MvTensorMyriadResources * myriadResourcePool, t_MvMatMulMyriadResources* matmulResourcesPool,
        t_mvTensorGenData * inputStruct, t_mvTensorGenData * weightsStruct, t_mvTensorGenData * biasStruct, t_mvTensorGenData * outputStruct,
        t_MvTensorOp * preOpStruct, t_MvTensorOp * opStruct , t_MvTensorOp * postOpStruct, t_MvTensorDebugInfo * dbgInfoArray, FathomRunConfig * fathomResources,
        short first_shave, short last_shave, void * inputTensor, void * outputTensor,
        unsigned int cache_memory_size, unsigned int scratch_memory_size,
        char *cache_memory_ptr, char *scratch_memory_ptr, unsigned int *offset_tracker ,int *piMaxOffset){
        int iMaxOffset = 0;
    for(unsigned int stage = 0; stage < total_stages; stage++){
        //Name
        unsigned char * StageName = readBlobString(&blobNetworkStart[*offset_tracker], BLOB_STRING_LENGTH, offset_tracker);

        //StageType
        opStruct[stage].type  = (t_MvTensorOpType)readBlob(&blobNetworkStart[*offset_tracker], &x_char, offset_tracker);
        opStruct[stage].optMask  = (t_MvTensorOpType)readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        opStruct[stage].radixX = (int)(readBlob(&blobNetworkStart[*offset_tracker], &x_char, offset_tracker));
        opStruct[stage].radixY = (int)readBlob(&blobNetworkStart[*offset_tracker], &x_char, offset_tracker);
        opStruct[stage].strideX = (int)readBlob(&blobNetworkStart[*offset_tracker], &x_char, offset_tracker);
        opStruct[stage].strideY = (int)readBlob(&blobNetworkStart[*offset_tracker], &x_char, offset_tracker);

        //padStyle
        opStruct[stage].padX = (t_MvTensorPaddStyle)readBlob(&blobNetworkStart[*offset_tracker], &x_char, offset_tracker);
        opStruct[stage].padY = (t_MvTensorPaddStyle)readBlob(&blobNetworkStart[*offset_tracker], &x_char, offset_tracker);
        opStruct[stage].paddStyle = (t_MvTensorPaddStyle)readBlob(&blobNetworkStart[*offset_tracker], &x_char, offset_tracker);

        opStruct[stage].opX = 0;

        // Dimensions for In, Taps, Out
        inputStruct[stage].dimX = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        inputStruct[stage].dimY = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        inputStruct[stage].dimZ = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);

        weightsStruct[stage].dimX = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        weightsStruct[stage].dimY = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        weightsStruct[stage].dimZ = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);

        outputStruct[stage].dimX = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        outputStruct[stage].dimY = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        outputStruct[stage].dimZ = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);

        // Strides for those same buffers
        inputStruct[stage].dimXStride = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        inputStruct[stage].dimYStride = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        inputStruct[stage].dimZStride = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);

        weightsStruct[stage].dimXStride = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        weightsStruct[stage].dimYStride = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        weightsStruct[stage].dimZStride = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);

        outputStruct[stage].dimXStride = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        outputStruct[stage].dimYStride = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        outputStruct[stage].dimZStride = readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);

#if 0
        DPRINTF("Input X Y Z DIMs:  %i %i %i\n",inputStruct[stage].dimX, inputStruct[stage].dimY, inputStruct[stage].dimZ);
        DPRINTF("Weight X Y Z DIMs:  %i %i %i\n",weightsStruct[stage].dimX, weightsStruct[stage].dimY, weightsStruct[stage].dimZ);
        DPRINTF("Output X Y Z DIMs:  %i %i %i\n",outputStruct[stage].dimX, outputStruct[stage].dimY, outputStruct[stage].dimZ);
        DPRINTF("Input X Y Z Strides:  %i %i %i\n",inputStruct[stage].dimXStride, inputStruct[stage].dimYStride, inputStruct[stage].dimZStride);
        DPRINTF("Weight X Y Z Strides:  %i %i %i\n",weightsStruct[stage].dimXStride, weightsStruct[stage].dimYStride, weightsStruct[stage].dimZStride);
        DPRINTF("Output X Y Z Strides:  %i %i %i\n\n",outputStruct[stage].dimXStride, outputStruct[stage].dimYStride, outputStruct[stage].dimZStride);
#endif

        //dataType
        readBlob(&blobNetworkStart[*offset_tracker], &x_char, offset_tracker); // Read input datatype (we don't assign it just yet)
        inputStruct[stage].dataType = t_fp16;
        weightsStruct[stage].dataType = t_fp16;
        outputStruct[stage].dataType = t_fp16;

        //InternalPrecision
        readBlob(&blobNetworkStart[*offset_tracker], &x_char, offset_tracker); // Read internal precision datatype (we don't assign it just yet)

        //storageOrder
        switch(readBlob(&blobNetworkStart[*offset_tracker], &x_char, offset_tracker)){
            default:  //orderYXZ
                inputStruct[stage].storageOrder = orderYXZ;
                weightsStruct[stage].storageOrder = orderXYZ;   //TODO: Are these valid?
                outputStruct[stage].storageOrder = orderYXZ;
                if(inputStruct[stage].dataType == 0){
                    inputStruct[stage].storageOrder = orderYXZ;
                    weightsStruct[stage].storageOrder = orderXYZ;   //TODO: Are these valid?
                    outputStruct[stage].storageOrder = orderYXZ;
                }
                break;
        }

        //The meaning of index is now this:
        // 0 - No data, pointer will be 0
        // 1 - Pointer is an offset into inputTensor
        // 2 - Pointer is an offset into outputTensor
        // 3 - Pointer is an offset into blob data
        // 4 - Pointer is an offset into fathomBSS
        //input pointer
        void *ptrs[4] = {inputTensor, outputTensor, blobNetworkStart + offset_to_weights, fathomResources->fathomBSS};
        unsigned int iPointer = (unsigned int)readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        short iIndex = (short)readBlob(&blobNetworkStart[*offset_tracker], &x_short, offset_tracker);
        if(iIndex > 4)  // Indexes > 4 have the same meaning of 4 (offset into fathomBSS)
            iIndex = 4;   // They exist for the new hardware, they are not useful for us
        inputStruct[stage].data = iIndex >= 1 && iIndex <= 4 ? (char *)ptrs[iIndex-1] + iPointer : 0;

        //tap pointer
        unsigned int wPointer = (unsigned int)readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        short wIndex = (short)readBlob(&blobNetworkStart[*offset_tracker], &x_short, offset_tracker);
        if(wIndex > 4)
            wIndex = 4;
        weightsStruct[stage].data = wIndex >= 1 && wIndex <= 4 ?(char *)ptrs[wIndex-1] + wPointer : 0;

        //bias pointer
        unsigned int biasPointer = (unsigned int)readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        short biasIndex = (short)readBlob(&blobNetworkStart[*offset_tracker], &x_short, offset_tracker);
        if(biasIndex > 4)
            biasIndex = 4;
        biasStruct[stage].data = biasIndex >= 1 && biasIndex <= 4 ? (char *)ptrs[biasIndex-1] + biasPointer : 0;

        // Parameters pointer
        unsigned int opParamsPointer = (unsigned int)readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        short opParamsIndex = (short)readBlob(&blobNetworkStart[*offset_tracker], &x_short, offset_tracker);
        if(opParamsIndex > 4)
            opParamsIndex = 4;
        opStruct[stage].params = opParamsIndex >=1 && opParamsIndex <= 4 ? (void *)((char *)ptrs[opParamsIndex - 1] + opParamsPointer) : 0;

	//output pointer
        unsigned int oPointer = (unsigned int)readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        short oIndex = (short)readBlob(&blobNetworkStart[*offset_tracker], &x_short, offset_tracker);
        if(oIndex > 4)
            oIndex = 4;
        outputStruct[stage].data = oIndex >= 1 && oIndex <= 4 ? (char *)ptrs[oIndex-1] + oPointer : 0;
        if (oIndex == 4 && 1024*1024 + oPointer + 3*sizeof(fp16)*(outputStruct[stage].dimX * outputStruct[stage].dimY *outputStruct[stage].dimZ ) > iMaxOffset)
	{
            iMaxOffset = 1024*1024 + oPointer + 3*sizeof(fp16)*(outputStruct[stage].dimX * outputStruct[stage].dimY *outputStruct[stage].dimZ ); 
	}

#if 0//DEBUG
        DPRINTF("INPUT OFFSET: %x (offset of %u)\n", inputStruct[stage].data, iPointer);
        DPRINTF("WEIGHT OFFSET: %x (offset of %u)\n", weightsStruct[stage].data, wPointer);
        DPRINTF("Output Location: %x (offset of %u) \n", outputStruct[stage].data, oPointer);
        DPRINTF("NETWORKS START: %x buff_offset %x = %p + %i\n",  blobNetworkStart , offset_to_weights, blobNetworkStart+offset_to_weights, oPointer);
#endif

        //StageType
        preOpStruct[stage].type  = (t_MvTensorOpType)readBlob(&blobNetworkStart[*offset_tracker], &x_char, offset_tracker);
        DPRINTF("PRE OP: %i\n",postOpStruct[stage].type );

        preOpStruct[stage].radixX = (int)1;
        preOpStruct[stage].radixY = (int)1;
        preOpStruct[stage].strideX = (int)1;
        preOpStruct[stage].strideY = (int)1;
        preOpStruct[stage].opX = (int)0;

        postOpStruct[stage].type  = (t_MvTensorOpType)readBlob(&blobNetworkStart[*offset_tracker], &x_char, offset_tracker);
        DPRINTF("POST OP: %i #%i\n",postOpStruct[stage].type, offset_tracker );
        unsigned int post_param1 = (unsigned int)readBlob(&blobNetworkStart[*offset_tracker], &x_uint, offset_tracker);
        // I am brutally deceiving the compiler here, but I am tired of warnings
        size_t ptr = (size_t)&post_param1;
        postOpStruct[stage].opX = *(float *)ptr;

        postOpStruct[stage].radixX = (int)1;
        postOpStruct[stage].radixY = (int)1;
        postOpStruct[stage].strideX = (int)readBlob(&blobNetworkStart[*offset_tracker], &x_short, offset_tracker);
        postOpStruct[stage].strideY = (int)readBlob(&blobNetworkStart[*offset_tracker], &x_short, offset_tracker);

        fathomNet[stage].input = &inputStruct[stage];
        fathomNet[stage].weights = &weightsStruct[stage];
        fathomNet[stage].output = &outputStruct[stage];
        fathomNet[stage].biases = &biasStruct[stage]; // todo connect this to fathom
#if 0
        if (stage == 25 || stage == 27 || stage == 29)
        {if (last_shave == 5)last_shave = 3;
         if (last_shave == 11)last_shave = 9;}
#endif
        int ns = last_shave - first_shave + 1;
        if(opStruct[stage].optMask & 0x78000000)
        {
            int ns1 = (opStruct[stage].optMask & 0x78000000) >> 27;
            if(ns1 <= ns)
                ns = ns1;
            else mvTensorAssert(0,"Number of shaves required in optMask for this layer is bigger then number of allocated shaves!");
        } else if((opStruct[stage].type == kAvgPool || opStruct[stage].type == kMaxPool) && ns > 2)
            ns = 2;
        else if(opStruct[stage].type == kFC)
            ns = 1;
        else if(opStruct[stage].type == kSoftMax)
        {
            int *axis = reinterpret_cast<int *>(opStruct[stage].params);
            mvTensorAssert(axis != NULL, "Softmax axis parameter = NULL");
            if(*axis == 1)
                ns = 1;
        }
        fathomNet[stage].myriadResources = configureMyriad(first_shave, first_shave + ns - 1, fathomResources->dmaLinkAgent,
                fathomResources->dataPartitionNo,
                fathomResources->instrPartitionNo,
                &myriadResourcePool[stage],
                (MvTensorDmaDescriptor*)fathomResources->dmaTransactions);

        fathomNet[stage].matmulResources = configureMatMulResources(cache_memory_size, scratch_memory_size,
                cache_memory_ptr, scratch_memory_ptr,
                &matmulResourcesPool[stage]);

        fathomNet[stage].preOp = &preOpStruct[stage];
        fathomNet[stage].op = &opStruct[stage];
        fathomNet[stage].postOp = &postOpStruct[stage];

        fathomNet[stage].debugInfo = &dbgInfoArray[stage];
    }
    *piMaxOffset = iMaxOffset;
#if 1 //DEBUG
    DPRINTF("Finished Parsing\n");
#endif

}

t_MvTensorMyriadResources * configureMyriad(int firstShave, int lastShave, int dmaAgent, int dataPartition,
        int instrPartition, t_MvTensorMyriadResources* stageResources,
        MvTensorDmaDescriptor* dmaTransactions){
    stageResources->firstShave = firstShave;
    stageResources->lastShave = lastShave;
    stageResources->dmaLinkAgent = dmaAgent;
    stageResources->dataPartitionNo = dataPartition;
    stageResources->instrPartitionNo = instrPartition;
    stageResources->dmaTransactions = dmaTransactions;
    return stageResources;
}

t_MvMatMulMyriadResources * configureMatMulResources(unsigned int cache_memory_size, unsigned int scratch_memory_size,
        char *cache_memory_ptr, char *scratch_memory_ptr,
        t_MvMatMulMyriadResources* matmulResources){
    matmulResources->cache_memory_ptr = cache_memory_ptr;
    matmulResources->scratch_memory_ptr = scratch_memory_ptr;
    matmulResources->cache_memory_size = cache_memory_size;
    matmulResources->scratch_memory_size = scratch_memory_size;
    return matmulResources;
}

/*
 * location: area to read from.
 * data_type: variable to represent T
 * size: optional variable to just read N number of characters.
 * TODO: Remove T parameter and specify in <T>. Also cast.
 */
    template <typename T>
T readBlob(unsigned char * location, T * data_type, unsigned int *offset)
{
    int data_sz = sizeof(T);
    (void)data_type;

    //Special Cases:
    if (data_sz == 1){  // It's a char, so just return the first item.
        *offset = *offset+1;
        return location[0];
    }

    unsigned char buffer[data_sz];

    T parts[data_sz];
    T combined = 0;

    for(int byte = 0; byte != data_sz; byte++){
        buffer[byte] = location[byte];
        parts[byte] = ((T)buffer[byte]) << (8*byte);
        combined = combined | parts[byte];
    }
    *offset = *offset + data_sz;
    return combined;
}

unsigned char * readBlobString(unsigned char * location, int size, unsigned int *offset){
    *offset =*offset+ size;
    return location;
}

