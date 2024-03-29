/* 
 * File:   atecc.h
 * Author: makmorit
 *
 * Created on 2020/08/12, 11:19
 */
#ifndef ATECC_H
#define ATECC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 使用するスロット番号の定義
//
//  8: 機密データを収容
// 13: AESパスワードを収容（キーハンドル／クレデンシャルIDの暗号／復号化用）
// 14: FIDO認証器固有の秘密鍵を収容
// 15: 13番・14番スロットに暗号化書込みするための一時キーを収容
//
#define KEY_ID_FOR_CONFIDENTIAL_DATA    8
#define KEY_ID_FOR_INSTALL_AES_PASSWORD 13
#define KEY_ID_FOR_INSTALL_PRIVATE_KEY  14
#define KEY_ID_FOR_INSTALL_PRV_TMP_KEY  15

//
// 関数群
//
bool     atecc_is_available(void);
bool     atecc_initialize(void);
void     atecc_finalize(void);
char    *atecc_get_serial_num_str(void);
bool     atecc_get_config_bytes(void);
bool     atecc_setup_config(void);
bool     atecc_install_privkey(uint8_t *privkey_raw_data);
bool     atecc_install_aes_password(uint8_t *aes_key_data, size_t aes_key_size);
bool     atecc_generate_pubkey_from_privkey(uint8_t *public_key_buff);
bool     atecc_generate_sign_with_privkey(uint16_t key_id, uint8_t const *hash_digest, uint8_t *signature);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_H */
