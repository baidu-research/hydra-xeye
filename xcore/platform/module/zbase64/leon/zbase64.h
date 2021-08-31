#ifndef ZBASE64_HPP
#define ZBASE64_HPP
#include <string>

using namespace std;

class ZBase64 {
public:
    std::string Encode(const unsigned char* Data,int DataByte);
    std::string Decode(const char* Data, int DataByte, int& OutByte);
private:
    std::size_t constexpr encoded_size(std::size_t len) {
        return 4 * ((len + 2) / 3);
    }
    std::size_t constexpr decoded_size(std::size_t len) {
        return (len * 3) >> 2;
    }
};

#endif  // ZBASE64_HPP
