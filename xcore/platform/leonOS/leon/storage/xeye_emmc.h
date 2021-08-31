/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef _XEYE_EMMC_H
#define _XEYE_EMMC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mv_types.h"
#define EMMC_MAGIC 0xEEEECCCC
#define BLOCK_SIZE 512
#define ONE_MEGA  (1024*1024)
#define ONE_GIGA  (1024*1024*1024)
#define BOOT_PARTITION    0
#define HEADER_PARTITION  (8*ONE_MEGA)
#define MODEL_PARTITION   (64*ONE_MEGA)
// NOTE(hyx): Becareful when the value is greater than max of int32,
// in that case, we must convert the value to u64 explicityly
#define HEADER_PARTITION  (8*ONE_MEGA)
#define MODEL_PARTITION   (64*ONE_MEGA)
#define CONFIG_PARTITION  (2*1024*1024*1024ULL)
#define IMAGE_PARTITION   (3*1024*1024*1024ULL)
#define CUSTOM_PARTITION  (6*1024*1024*1024ULL)

#define MAX_MODEL_NUM  100
#define MAX_CONFIG_NUM 100
#define MAX_IMAGE_NUM  100000
#define MAX_CUSTOM_NUM 1000

typedef struct EmmcFile {
    u64 addr;
    u32 length;
    u8  name[32];
} EmmcFile_t;

typedef struct EmmcHeader {
    u32 magicNum;
    u32 loading_model_count;
    char loading_model_name[10][32];
    u32 model_count;
    EmmcFile_t model[MAX_MODEL_NUM];
    u32 config_count;
    EmmcFile_t config[MAX_CONFIG_NUM];
    u32 image_count;
    EmmcFile_t image[MAX_IMAGE_NUM];
    u32 custom_count;
    EmmcFile_t custom[MAX_CUSTOM_NUM];
} EmmcHeader_t;

typedef enum EmmcPartition {
    BOOT_PART = 0,
    HEADER_PART,
    MODEL_PART,
    CONFIG_PART,
    IMAGE_PART,
    CUSTOM_PART,
    ALL_PARTS,
} EmmcPartition_t;

typedef struct EmmcUpdateFile {
    char name[32];
    u32 type;
    u32 addr;
    u32 len;
    u32 fresh;
    u32 boot;
} UpdateFileHeader_t;

typedef struct EmmcUpdateFileHeader {
    unsigned int magic;
    int count;
    UpdateFileHeader_t list[10];
} UpdateFileListHeader_t;

typedef struct FilesHash {
    unsigned char graphlist_hash[16];
    unsigned char config_hash[16];
    unsigned char models_name[10][32];
    unsigned char models_hash[10][16];
    int models_num;
} FilesHash_t;

// static function
int XeyeDrvReadEmmc(u64 from, u32* to, u32 length);
int XeyeDrvWriteEmmc(u64 to, u32* from, u32 length);
int XeyeDrvEmmcInit(void);
u32 XeyeDrvEmmcOpAlignSize(u32 size, u32 align_bit);
int XeyeDrvEmmcClean(int part);
int XeyeDrvEmmcGetFileFromPartition(const char* name, int part, unsigned char* buf, int* len);
int XeyeDrvEmmcGetHeader(unsigned char* buf);
int EmmcUpdateFiles(char* file_list, int part);
char* EmmcPartText(int part);
int XeyeDrvEmmcPrintInfo(void);
int EmmcUpdateFileFromMemory(void* mem, const char* name, u32 len, int part, int fresh);
int XeyeUpdateModelAndConfig(void);

// extern function
int get_file_from_emmc(char* name, int part, char** file_buffer, int* length);
int get_graphlist_from_emmc(char** graphlist, int* length);
int get_config_file_from_emmc(char* config_file, int* length);
int get_config_file(char* config_file, int* length);
int read_all_graph_from_emmc(void);
unsigned char*  get_graphlist_md5(void);
FilesHash_t* get_emmc_files_hash(void);
int XeyeDrvEmmcMountFat32(int format);
int XeyeGetModelsFromEmmc(int encrypt);

#ifdef __cplusplus
}
#endif
#endif
