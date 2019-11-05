#ifndef FIDO_FLASH_H__
#define FIDO_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "fds.h"

// Flash ROMに保存するための
// ファイルID、レコードKey
//
// File IDs should be in the range 0x0000 - 0xBFFF.
// Record keys should be in the range 0x0001 - 0xBFFF.
// https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v15.0.0/lib_fds_functionality.html
//
//  鍵・証明書管理用
//
#define FIDO_SKEY_CERT_FILE_ID        (0xBFFE)
#define FIDO_SKEY_CERT_RECORD_KEY     (0xBFEE)
//
//  ペアリングモード管理用
//
#define FIDO_PAIRING_MODE_FILE_ID     (0xBFFD)
#define FIDO_PAIRING_MODE_RECORD_KEY  (0xBFED)
//
//  AESパスワード管理用
//
#define FIDO_AESKEYS_FILE_ID          (0xBFFC)
#define FIDO_AESKEYS_RECORD_KEY       (0xBFEC)
//
//  トークンカウンター管理用
//  レコードサイズ = 17 ワード
//    レコードキー: 8ワード（32バイト）
//      U2Fの場合＝appIdHash
//      CTAP2の場合＝Public Key Credential Sourceから生成されたSHA-256ハッシュ値
//      （実質、Webサイト＋ユーザー＋鍵の組み合わせでユニークとなるキー）
//    トークンカウンター: 1ワード（4バイト)
//    Webサイト情報: 8ワード（32バイト）
//      U2Fの場合＝appIdHash
//      CTAP2の場合＝rpIdHash
#define FIDO_TOKEN_COUNTER_FILE_ID    (0xBFFB)
#define FIDO_TOKEN_COUNTER_RECORD_KEY (0xBFEB)
#define FIDO_TOKEN_COUNTER_RECORD_SIZE 17
//
//  PINリトライカウンター管理用
//  トークンカウンター管理と同一ファイルで管理
//  レコードサイズ = 9 ワード
//    PINコードハッシュ: 8ワード（32バイト）
//    リトライカウンター: 1ワード（4バイト)
#define FIDO_PIN_RETRY_COUNTER_FILE_ID      (FIDO_TOKEN_COUNTER_FILE_ID)
#define FIDO_PIN_RETRY_COUNTER_RECORD_KEY   (0xBFEA)
#define FIDO_PIN_RETRY_COUNTER_RECORD_SIZE  9
//
// BLEペリフェラルによる自動認証パラメーター管理用
//   レコードサイズ = 11 ワード
//     スキャン対象サービスUUID文字列: 9ワード（36バイト)
//     スキャン秒数: 1ワード（4バイト）
//     自動認証フラグ: 1ワード（4バイト）
#define FIDO_BLP_AUTH_PARAM_FILE_ID         (0xBFF9)
#define FIDO_BLP_AUTH_PARAM_RECORD_KEY      (0xBFE9)
#define FIDO_BLP_AUTH_PARAM_RECORD_SIZE     11

//
//  鍵・証明書の長さを管理
//
#define SKEY_WORD_NUM 8
#define CERT_WORD_NUM 257
#define SKEY_CERT_WORD_NUM (SKEY_WORD_NUM+CERT_WORD_NUM)

//
// fido_flash_event.c
//
void fido_flash_fds_event_register(void);
void fido_flash_fds_force_gc(void);
bool fido_flash_fds_record_get(fds_record_desc_t *record_desc, uint32_t *record_buffer);

//
// fido_flash_pairing_mode.c
//
bool fido_flash_pairing_mode_flag(void);
void fido_flash_pairing_mode_flag_set(void);
void fido_flash_pairing_mode_flag_clear(void);

#ifdef __cplusplus
}
#endif

#endif // FIDO_FLASH_H__

/** @} */
