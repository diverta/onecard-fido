#ifndef U2F_FLASH_H__
#define U2F_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// Flash ROMに保存するための
// ファイルID、レコードKey
//
//  鍵・証明書・トークンカウンター管理用
//
#define U2F_FILE_ID                  (0xBFFE)
#define U2F_SKEY_CERT_RECORD_KEY     (0xBFFE)
#define U2F_TOKEN_COUNTER_RECORD_KEY (0xBFFD)
//
//  ペアリングモード管理用
//
#define U2F_PAIRING_FILE_ID          (0xBFFD)
#define U2F_PAIRING_MODE_RECORD_KEY  (0xBFFE)
//
//  AESパスワード管理用
//
#define U2F_AESKEYS_FILE_ID          (0xBFFC)
#define U2F_AESKEYS_MODE_RECORD_KEY  (0xBFFE)

uint32_t *u2f_flash_keydata_buffer(void);

bool u2f_flash_force_fdc_gc(void);
bool u2f_flash_keydata_delete(void);
bool u2f_flash_keydata_read(void);
bool u2f_flash_keydata_available(void);
bool u2f_flash_keydata_write(void);
bool u2f_flash_token_counter_write(uint8_t *p_appid_hash, uint32_t token_counter, uint32_t reserve_word);
bool u2f_flash_token_counter_read(uint8_t *p_appid_hash);
uint32_t u2f_flash_token_counter_value(void);

#ifdef __cplusplus
}
#endif

#endif // U2F_FLASH_H__

/** @} */
