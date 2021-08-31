/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_FILE_MAP_H
#define BAIDU_XEYE_FILE_MAP_H

#include <string>

class FileMap {
public:
    FileMap();
    ~FileMap();

    int init(const std::string &file_path);
    void * map();
    uint32_t size();

private:
    void uninit();
    
    std::string _file_path;
    void *_file_map;
    uint32_t _file_size;
};

#endif // BAIDU_XEYE_FILE_MAP_H
