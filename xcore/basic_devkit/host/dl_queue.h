/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef BAIDU_XEYE_DL_QUEUE_H
#define BAIDU_XEYE_DL_QUEUE_H

#include <stddef.h>

#include <mutex>
#include <queue>

template<typename T, typename Container = std::queue<T> >
class DlQueue {
public:
    DlQueue(size_t max_queue_size) {
        size_t default_queue_size = 8;

        if (0 == max_queue_size) {
            _max_queue_size = default_queue_size;
            printf("set max_queue_size to default_queue_size %lu\n",
                   default_queue_size);
        } else {
            _max_queue_size = max_queue_size;
        }
    }

    ~DlQueue() {
    }

    int push(T &item) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.size() >= _max_queue_size) {
            //printf("exceed max_queue_size remove front.\n");
            return -1;
        }
        _queue.push(item);
        return 0;
    }

    int pop(T &item) {
        int ret = 0;
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.empty()) {
            ret = -1;
            // printf("queue is empty.\n");
        } else {
            item = _queue.front();
            _queue.pop();
        }
        return ret;
    }

private:
    Container _queue;
    size_t _max_queue_size;
    std::mutex _mutex;
};

#endif // BAIDU_XEYE_DL_QUEUE_H
