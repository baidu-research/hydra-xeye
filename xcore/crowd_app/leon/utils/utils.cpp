#include "utils.hpp"

namespace utils {
uint64_t unix_timestamp() {
    std::chrono::time_point<std::chrono::system_clock> p1;
    p1 = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
        p1.time_since_epoch()).count();
}

}
