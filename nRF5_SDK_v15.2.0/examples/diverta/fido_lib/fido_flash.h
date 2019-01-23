#ifndef FIDO_FLASH_H__
#define FIDO_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

// Flash ROMに保存するための
// ファイルID、レコードKey
//
//  鍵・証明書管理用
//
#define FIDO_SKEY_CERT_FILE_ID        (0xBFFE)
#define FIDO_SKEY_CERT_RECORD_KEY     (0xBFFE)
//
//  ペアリングモード管理用
//
#define FIDO_PAIRING_MODE_FILE_ID     (0xBFFD)
#define FIDO_PAIRING_MODE_RECORD_KEY  (0xBFFE)
//
//  AESパスワード管理用
//
#define FIDO_AESKEYS_FILE_ID          (0xBFFC)
#define FIDO_AESKEYS_RECORD_KEY       (0xBFFE)
//
//  トークンカウンター管理用
//
#define FIDO_TOKEN_COUNTER_FILE_ID    (0xBFFB)
#define FIDO_TOKEN_COUNTER_RECORD_KEY (0xBFFD)

//
//  鍵・証明書の長さを管理
//
#define SKEY_WORD_NUM 8
#define CERT_WORD_NUM 257
#define SKEY_CERT_WORD_NUM (SKEY_WORD_NUM+CERT_WORD_NUM)

bool      fido_flash_force_fdc_gc(void);

bool      fido_flash_skey_cert_delete(void);
bool      fido_flash_skey_cert_write(void);
bool      fido_flash_skey_cert_read(void);
bool      fido_flash_skey_cert_available(void);
uint32_t *fido_flash_skey_cert_data(void);

bool      fido_flash_token_counter_delete(void);
bool      fido_flash_token_counter_write(uint8_t *p_appid_hash, uint32_t token_counter, uint32_t reserve_word);
bool      fido_flash_token_counter_read(uint8_t *p_appid_hash);
uint32_t  fido_flash_token_counter_value(void);

#ifdef __cplusplus
}
#endif

#endif // FIDO_FLASH_H__

/** @} */
