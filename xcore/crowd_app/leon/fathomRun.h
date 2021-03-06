/*
 * fathom.h
 *
 *  Created on: Jun 24, 2016
 *      Author: ian-movidius
 */

///
/// @file
/// @copyright All code copyright Movidius Ltd 2016, all rights reserved
///            For License Warranty see: common/license.txt
///
/// @defgroup Fathom
/// @{
/// @brief    FathomRun component API.
///
/// The FathomRun API is uesd to run a pre-generated CNN blob. The blob contains the whole neural
/// network together with weights. The blob is parsed and translated into MvTensor calls which
/// run on Shaves.
///
/// @par Component Usage
///  In order to use the component the following steps are ought to be done:
///  1. Generate a CNN blob using the Fathom tool.
///  2. Integrate the FathomRun component into your project. Study the Classify example for
///  a simple integration example.
///  3. Build and run the project and observe the output generated after running the blob.
///
///@}
/// @defgroup Fathom
/// @ingroup Fathom
/// @{
/// @brief FathomRun component API.
///

#ifndef _FATHOMRUN_H_
#define _FATHOMRUN_H_

#include <mvTensor.h>
#include "pubdef.h"
#include "resourceshare.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BLOB_FILE_SIZE_OFFSET 32

typedef void (*FathomRunCallback)(u32 stageNumber);


/// @brief FathomRun component
/// @param[in] blob - The blob that contains the neural network, generated by Fathom.
/// @param[in] blob_size - The blob's size in bytes.
/// @param[in] inputTensor - The input image to the network.
/// @param[out] outputTensor - The output of the blob run. It can be anything, depending on
/// what the network computes: an images, multiple images, an array of values (classification label) etc.
/// This is meant to be parsed and interpreted according to its meaning.
/// @param[in] fathomResource - A structure containing info about system resources used: memory,
/// DMA etc.
/// @param[out] timings - An array containing MvTensor calls recorded run-times, in the order
/// defined in the blob. Each entry contains a single number of milliseconds regardless of
/// the number of Shaves used.
/// @param[in] print_times - Optional paramer. Debug flag to print MvTensor calls. This has effect only when the
/// application is run through the JTAG. When Fathom is run over USB no output will be shown unless
/// the PC side grabs and displays it.
/// @param[in] cache_memory_size - Size of cache memory used by MatMul
/// @param[in] scratch_memory_size - Size of scratch memory used by MatMul
/// @param[in] cache_memory_ptr - Pointer to a DDR location that will be use by MatMul (recomanded size is 25MB)
/// @param[in] scratch_memory_ptr - Pointer to a CMX location that will be use by MatMul (recomanded size is 110KB)
/// @param[in] debug - Optional paramer. Debug flag which enables more verbose output. Same behaviour as the print_times flag.
/// @param[in] cb - Optional paramer. Callback which is executed after each MvTensor call. The only parameter it
/// offers will be initialized with the MvTensor call number. This callback can be used in various ways:
/// debugging, synchronization with other components, profiling etc.
/// @return u32 Always returns 0 for now. It might be used to return error codes in the future.
///
t_MvTensorMyriadResources* configureMyriad(int firstShave, int lastShave, int dmaAgent,
        int dataPartition, int instrPartition, t_MvTensorMyriadResources* stageResources, MvTensorDmaDescriptor* dmaTransactions);
int FathomInitModel(t_ModelParam * modelParam,unsigned char * blob, int blob_size, void * inputTensor, void * outputTensor,
    FathomRunConfig * fathomResources,  u8 * debugBuffer,
    unsigned int cache_memory_size, unsigned int scratch_memory_size,
    char *cache_memory_ptr, char *scratch_memory_ptr, int print_times, int debug);

void fathomRun(t_ModelParam * modelParam);

//int fathom_init(void);
/// @}
#ifdef __cplusplus
}
#endif

#endif /* _FATHOMRUN_H_ */
