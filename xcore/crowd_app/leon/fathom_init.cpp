///
/// @file
/// @copyright All code copyright Movidius Ltd 2012, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     MvTensor Test application
///


// Includes
// ----------------------------------------------------------------------------
#include <string.h>
#include <stdio.h>
#include <rtems.h>
#include <UnitTestApi.h>
#include <VcsHooksApi.h>
#include <stdlib.h>
#include <assert.h>
#include <swcShaveLoader.h>
#include <Fp16Convert.h>
#include <DrvLeonL2C.h>

#include "mv_types.h"
#include "mvHelpersApi.h"
#include "resourceshare.h"
//#include "definecpp.h"
#include "define.h"
#include "fathomRun.h"
#include "pubdef.h"
#include "fathom_init.h"
#include "params/params.hpp"
#include "graph.hpp"
#include <vector>
#ifndef MVLOG_UNIT_NAME
#define MVLOG_UNIT_NAME fathom
#endif
#include <mvLog.h>
#include <fcntl.h>
#include <cassert>
#include <fstream>
#include "utils/utils.hpp"

// Source Specific #defines and types  (typedef,enum,struct)
// ----------------------------------------------------------------------------
#define DDR_DATA  __attribute__((section(".ddr.data")))
#define DDR_BSS  __attribute__((section(".ddr.bss")))
#define DMA_DESCRIPTORS_SECTION __attribute__((section(".cmx.cdmaDescriptors")))

#define FATHOM_BUFFER_SIZE (60*1024*1024)
#define SIGNLE_FATHOM_SIZE (30*1024*1024)
#define FATHOM_INPUT_SIZE  1448576

u8 DDR_BSS fathomBSS[FATHOM_BUFFER_SIZE] __attribute__((aligned(64)));
static int g_modelNum = 0;
extern u32 lrt_modelNum;

uint8_t debugMsg[MAX_GRAPH][MV_TENSOR_DBG_MSG_SIZE];

const unsigned int cache_memory_size = 22 * 1024 * 1024;
const unsigned int scratch_memory_size = 20 * 1024;

char DDR_BSS cache_memory[MAX_GRAPH][cache_memory_size];
char __attribute__((section(".cmx.bss"))) scratch_memory[MAX_GRAPH][scratch_memory_size];

// t_graph lrt_g_graphList[MAX_GRAPH];
std::vector<Graph_t> g_graphs;
extern std::vector<t_ModelParam> lrt_g_modelParam;

MvTensorDmaDescriptor DMA_DESCRIPTORS_SECTION task[MAX_GRAPH];

static bool read_graph_from_disk(const std::string& model_abs_path, \
                                 Graph_t* graph) {
    assert(graph != NULL);
    assert(!model_abs_path.empty());
    std::ifstream ifs(model_abs_path, std::ifstream::binary | std::ifstream::ate);
    if (!ifs.is_open()) {
        mvLog(MVLOG_ERROR, "can not open model graph file: %s", model_abs_path.c_str());
        return false;
    }
    std::ifstream::pos_type pos = ifs.tellg();
    int bytes = static_cast<int>(pos);
    if (bytes <= 0) {
        mvLog(MVLOG_ERROR, "invalid model graph file size: %d", bytes);
        return false;
    }
    graph->graph.reset(new char[bytes]);
    if (graph->graph == nullptr) {
        mvLog(MVLOG_ERROR, "can not allocate %d bytes for model graph", bytes);
        return false;
    }
    // move position to the beggining
    ifs.seekg(0, std::ios::beg);
    ifs.read(graph->graph.get(), bytes);
    graph->graph_len = bytes;
    return true;
}

bool init_graphs_from_params(std::vector<Graph_t>* graphs) {
    assert(graphs != NULL);
    std::shared_ptr<Params> params_instance = Params::getInstance();
    int total_models = params_instance->models_count();
    if (total_models <= 0) {
        mvLog(MVLOG_ERROR, "no models found in the configuration file.");
        return false;
    }
    graphs->resize(total_models);
    for (int i = 0; i < total_models; ++i) {
        Graph_t& this_graph = (*graphs)[i];
        std::string model_name = params_instance->get_model_name_by_index(i);
        std::string model_path = params_instance->get_model_path_by_index(i);
        const std::string model_abs_path = model_path + "/" + model_name;
        if (!read_graph_from_disk(model_abs_path, &this_graph)) {
            mvLog(MVLOG_ERROR, "can not read graph from disk");
            return false;
        }
        utils::MD5Calculator md5_calc;
        this_graph.md5 = md5_calc(reinterpret_cast<unsigned char*>(\
                    this_graph.graph.get()), size_t(this_graph.graph_len));
        this_graph.graph_name = model_name;
        mvLog(MVLOG_INFO, "Graph name: %s, Graph_path: %s, "\
                "Graph_size: %d, MD5: %s", \
                this_graph.graph_name.c_str(), model_path.c_str(), \
                this_graph.graph_len, this_graph.md5.c_str());
    }
    return true;
}

int initModelParam(int number, const Graph_t* graph, t_ModelParam* modelParam) {
    modelParam->graphLen = graph->graph_len;
    modelParam->graph = graph->graph.get();
    if (number == 0) {
         modelParam->fathomAgent = 0;
    } else if (number == 1) {
         modelParam->fathomAgent = 1;
    }
    return 0;
}

int getBssOffset(int index) {
    return lrt_g_modelParam[index].fathomBssLen;
}


int initModel(int number, Graph_t* graph, t_ModelParam* modelParam) {
    u32 FathomBlobSizeBytes;
    int ret = 0;

    ret = initModelParam(number, graph, modelParam);
    if (ret < 0) {
	    printf("model error!\n");
        return -1;
    }

    FathomBlobSizeBytes = *(u32*)&(graph->graph.get()[BLOB_FILE_SIZE_OFFSET]);

    char *fathomBuffer = (char *)fathomBSS + getBssOffset(number);
    char *deepInBuf = (char *)malloc(FATHOM_INPUT_SIZE);
    char *deepOutBuf = (char *)malloc(FATHOM_OUTPUT_SIZE);
    if (NULL == deepInBuf || NULL == deepOutBuf) {
        if( NULL != deepInBuf)
            free(deepInBuf);
        if(NULL != deepOutBuf)
            free(deepOutBuf);
        printf("No free memory for model init. number %d.\n",number);
        return -1;
    }

    modelParam->fathomBuf = fathomBuffer;
    modelParam->modelInBuf = deepInBuf;
    modelParam->index = number;
    debugMsg[number][0] = 0;
    FathomRunConfig config;
    if (modelParam->fathomAgent == 0){
      config =
      {
        .fathomBSS = fathomBSS,
        .fathomBSS_size = SIGNLE_FATHOM_SIZE,
        .dmaLinkAgent = 1,
        .dataPartitionNo = 0,
        .instrPartitionNo = 1,
        .firstShave = 0,
        .lastShave = MVTENSOR_MAX_SHAVES - 1,
        .dmaTransactions = &task[number]
      };
    }
    else{
      config =
      {
        .fathomBSS = fathomBSS+SIGNLE_FATHOM_SIZE,
        .fathomBSS_size = SIGNLE_FATHOM_SIZE,
        .dmaLinkAgent = 3,
        .dataPartitionNo = 2,
        .instrPartitionNo = 3,
        .firstShave = 0,
        .lastShave = MVTENSOR_MAX_SHAVES - 1,
        .dmaTransactions = &task[number]
      };

    }

    modelParam->fathomconfig = &config;
    ret = FathomInitModel(modelParam, reinterpret_cast<unsigned char*>(graph->graph.get()), \
            (u32)FathomBlobSizeBytes, deepInBuf, deepOutBuf, &config, debugMsg[number],
            cache_memory_size, scratch_memory_size, cache_memory[number], \
            scratch_memory[number], 1, 1);
    if (0 != ret){
        printf("init fathom error.\n");
    }
    return ret;
}

int fathom_init(void)
{
    int ret = 0;
    mvLog(MVLOG_INFO, "Begin to load model graph");
    if (!init_graphs_from_params(&g_graphs)) {
        mvLog(MVLOG_ERROR, "Failed to load model graph");
        return -1;
    }
    memset(fathomBSS, 0, sizeof(fathomBSS));

    assert(g_graphs.size() > 0);

    lrt_g_modelParam.resize(MAX_GRAPH);
    for(size_t i = 0; i < MAX_GRAPH; i++) {
        ret = initModel(i, &g_graphs[0], &lrt_g_modelParam[i]);
        if (ret < 0) {
            mvLog(MVLOG_INFO, "init model %d failed.\n", i);
            break;
        }
        printf("offset %d \n", getBssOffset(i));
    }
    return ret;
}

