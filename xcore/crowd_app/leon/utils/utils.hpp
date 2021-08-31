#ifndef MD5_UTILS_HPP
#define MD5_UTILS_HPP

#include <xeye_md5.h>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>
#include <iomanip>
namespace utils {
// get system timestamp
extern uint64_t unix_timestamp();


// utils for md5 calculator
struct MD5Calculator {
public:
    std::string operator()(const unsigned char* buf, size_t len) {
        unsigned char md5_res[16] = {0};
        md5(buf, len, md5_res);
        return convert_md5_result_to_hex_string(md5_res, 16);
    }
private:
    std::string convert_md5_result_to_hex_string(unsigned char md5_res[], size_t n) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (size_t i = 0; i < n; ++i) {
            ss << std::setw(2) << int(md5_res[i]);
        }
        return ss.str();
    }
};

}
#endif  // MD5_UTILS_HPP
