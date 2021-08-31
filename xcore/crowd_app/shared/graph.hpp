#ifndef _SHARED_GRAPH_H
#define _SHARED_GRAPH_H

#include <memory>
#include <string>

struct Graph_t {
    int graph_len;
    std::string graph_name;
    std::shared_ptr<char> graph;
    std::string md5;
};
#endif  // _SHARED_GRAPH_H
