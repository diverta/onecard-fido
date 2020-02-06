#ifndef U2F_CRYPTO_ECB_H__
#define U2F_CRYPTO_ECB_H__

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t *u2f_keyhandle_base_buffer(void);
uint8_t *u2f_keyhandle_buffer(void);
size_t   u2f_keyhandle_buffer_size(void);
void     u2f_keyhandle_generate(uint8_t *p_appid_hash);
void     u2f_keyhandle_restore(uint8_t *keyhandle_value, uint32_t keyhandle_length);
void     u2f_keyhandle_do_sign(void);

#ifdef __cplusplus
}
#endif

#endif // U2F_CRYPTO_ECB_H__

/** @} */
