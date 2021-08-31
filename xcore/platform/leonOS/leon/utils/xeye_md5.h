#ifndef _MD5_H
#define _MD5_H

#ifdef __cplusplus
extern "C" {
#endif

struct MD5Context {
    unsigned int buf[4];
    unsigned int bits[2];
    union {
        unsigned char in[64];
        unsigned int in32[16];
    };
};

void md5(const unsigned char* input, int len, unsigned char output[16]);
void md5_wd(const unsigned char* input, int len, unsigned char output[16],
            unsigned int chunk_sz);

#ifdef __cplusplus
}
#endif
#endif /* _MD5_H */
