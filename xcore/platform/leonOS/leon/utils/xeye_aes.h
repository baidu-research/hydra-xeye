#ifndef _XEYE_AES_H_
#define _XEYE_AES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <types.h>

#define debug(...) do {} while (0)
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

enum {
    AES_STATECOLS   = 4,
    AES_KEYCOLS = 4,
    AES_ROUNDS  = 10,
    AES_KEY_LENGTH  = 128 / 8,
    AES_EXPAND_KEY_LENGTH   = 4 * AES_STATECOLS * (AES_ROUNDS + 1),
};

void aes_expand_key(u8* key, u8* expkey);
void aes_encrypt(u8* in, u8* expkey, u8* out);
void aes_decrypt(u8* in, u8* expkey, u8* out);
void aes_apply_cbc_chain_data(u8* cbc_chain_data, u8* src, u8* dst);
void aes_cbc_encrypt_blocks(u8* key_exp, u8* src, u8* dst, u32 num_aes_blocks);
void aes_cbc_decrypt_blocks(u8* key_exp, u8* src, u8* dst, u32 num_aes_blocks);
int aes_cbc_crypt(char* payload, const int enc, uint8_t* key, const int len);

#ifdef __cplusplus
}
#endif
#endif /* _XEYE_AES_H_ */
