#ifndef U2F_CRYPTO_ECB_H__
#define U2F_CRYPTO_ECB_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


bool u2f_crypto_ecb_init(void);
void u2f_crypto_ecb_encrypt(uint8_t *packet, uint32_t packet_length, uint8_t *out_packet);
void u2f_crypto_ecb_decrypt(uint8_t *packet, uint32_t packet_length, uint8_t *out_packet);
void u2f_crypto_ecb_generate_keyhandle(uint8_t *p_appid_hash, uint8_t *private_key_value, uint32_t private_key_length);
void u2f_crypto_ecb_restore_keyhandle_base(uint8_t *keyhandle_value, uint32_t keyhandle_length);

// キーハンドル生成・格納用領域
// Register, Authenticateで共通使用
//   keyhandle_base_buffer
//     暗号化される前（＝復合化された後）のキーハンドル
//     この中に、AppIDHash、秘密鍵を格納
//   keyhandle_buffer
//     暗号化された後のキーハンドル。
//     U2Fサーバー／クライアントとの受け渡しに使用
extern uint8_t keyhandle_base_buffer[64];
extern uint8_t keyhandle_buffer[64];


#ifdef __cplusplus
}
#endif

#endif // U2F_CRYPTO_ECB_H__

/** @} */
