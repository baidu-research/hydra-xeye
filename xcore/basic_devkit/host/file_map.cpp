/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#include "file_map.h"

#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

FileMap::FileMap() {
    _file_map = NULL;
    _file_size = 0;
}

FileMap::~FileMap() {
    uninit();
}

int FileMap::init(const std::string &file_path) {
    if (file_path.empty()) {
        printf("FileMap::init failed. empty file_path.\n");
        return -1;
    }
    _file_path = file_path;

    int ret = 0;
    int fd = -1;
    do {
        fd = open((char *)_file_path.c_str(), O_RDONLY);
        if (-1 == fd) {
            printf("failed to open file %s\n", _file_path.c_str());
            ret = -2;
            break;
        }

        struct stat st;
        if (0 != fstat(fd, &st)) {
            printf("failed to fstat file %s\n", _file_path.c_str());
            ret = -3;
            break;
        }
        if (!(S_IFREG & st.st_mode)) {
            printf("file %s is not a regular file.\n", _file_path.c_str());
            ret = -4;
            break;
        }
        if (st.st_size < 8) {
            printf("file %s is too small.\n", _file_path.c_str());
            ret = -5;
            break;
        }
        _file_size = st.st_size;

        _file_map = mmap(NULL, _file_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (MAP_FAILED == _file_map) {
            printf("failed to mmap %s.\n", _file_path.c_str());
            ret = -6;
            break;
        }
    } while (false);

    if (-1 != fd) {
        close(fd);
        fd = -1;
    }
    if (0 != ret) {
        uninit();
    }
    return ret;
}

void FileMap::uninit() {
    _file_path.clear();
    if (NULL != _file_map) {
        munmap(_file_map, _file_size);
        _file_map = NULL;
    }
    _file_size = 0;
}

void * FileMap::map() {
    return _file_map;
}

uint32_t FileMap::size() {
    return _file_size;
}
