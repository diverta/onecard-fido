#ifndef FIDO_FLASH_H__
#define FIDO_FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

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
// 以下はPIVアプリケーションで使用
//
// PIVオブジェクト
//   オブジェクト属性 = 2ワード
//     属性データ: 1ワード（4バイト）
//       0    : 種別（1バイト）
//       1    : アルゴリズム（1バイト）
//       2 - 3: 予備（2バイト）
//     オブジェクトデータの長さ: 1ワード（4バイト）
//   オブジェクトデータ = 可変長（最大256ワード＝1,024バイト）
#define PIV_DATA_OBJ_FILE_ID            (0xBFDE)
#define PIV_DATA_OBJ_ATTR_WORDS         2
#define PIV_DATA_OBJ_DATA_WORDS_MAX     256
//
// PIVオブジェクト格納ファイルで
// 共通利用するレコードID
//
#define PIV_DATA_OBJ_9A_RECORD_KEY      (0xBFCE)
#define PIV_DATA_OBJ_9B_RECORD_KEY      (0xBFCD)
#define PIV_DATA_OBJ_9C_RECORD_KEY      (0xBFCC)
#define PIV_DATA_OBJ_9D_RECORD_KEY      (0xBFCB)
#define PIV_DATA_OBJ_02_RECORD_KEY      (0xBFCA)
#define PIV_DATA_OBJ_05_RECORD_KEY      (0xBFC9)
#define PIV_DATA_OBJ_07_RECORD_KEY      (0xBFC8)
#define PIV_DATA_OBJ_0A_RECORD_KEY      (0xBFC7)
#define PIV_DATA_OBJ_0B_RECORD_KEY      (0xBFC6)
#define PIV_DATA_OBJ_80_RECORD_KEY      (0xBFC5)
#define PIV_DATA_OBJ_81_RECORD_KEY      (0xBFC4)

//
// 以下はOpenPGPアプリケーションで使用
//
// OpenPGPオブジェクト
//   オブジェクトデータの長さ: 1ワード（4バイト）
//   オブジェクトデータ = 可変長（最大256ワード＝1,024バイト）
#define OPGP_DATA_OBJ_FILE_ID           (0xBFBE)
#define OPGP_DATA_OBJ_WORDS_MAX         256
//
// OpenPGPオブジェクト格納ファイルで
// 共通利用するレコードID
//
#define OPGP_DATA_OBJ_01_RECORD_KEY     (0xBFAE)
#define OPGP_DATA_OBJ_02_RECORD_KEY     (0xBFAD)
#define OPGP_DATA_OBJ_03_RECORD_KEY     (0xBFAC)
#define OPGP_DATA_OBJ_04_RECORD_KEY     (0xBFAB)
#define OPGP_DATA_OBJ_05_RECORD_KEY     (0xBFAA)
#define OPGP_DATA_OBJ_06_RECORD_KEY     (0xBFA9)
#define OPGP_DATA_OBJ_07_RECORD_KEY     (0xBFA8)
#define OPGP_DATA_OBJ_08_RECORD_KEY     (0xBFA7)
#define OPGP_DATA_OBJ_09_RECORD_KEY     (0xBFA6)
#define OPGP_DATA_OBJ_10_RECORD_KEY     (0xBFA5)
#define OPGP_DATA_OBJ_11_RECORD_KEY     (0xBFA4)
#define OPGP_DATA_OBJ_12_RECORD_KEY     (0xBFA3)
#define OPGP_DATA_OBJ_13_RECORD_KEY     (0xBFA2)
#define OPGP_DATA_OBJ_14_RECORD_KEY     (0xBFA1)
#define OPGP_DATA_OBJ_15_RECORD_KEY     (0xBF9E)
#define OPGP_DATA_OBJ_16_RECORD_KEY     (0xBF9D)
#define OPGP_DATA_OBJ_17_RECORD_KEY     (0xBF9C)
#define OPGP_DATA_OBJ_18_RECORD_KEY     (0xBF9B)
#define OPGP_DATA_OBJ_19_RECORD_KEY     (0xBF9A)
#define OPGP_DATA_OBJ_20_RECORD_KEY     (0xBF99)

//
// fido_flash_event.c
//
void fido_flash_event_set_gc_forced(void);
void fido_flash_fds_event_register(void);
void fido_flash_storage_init(void);

#ifdef __cplusplus
}
#endif

#endif // FIDO_FLASH_H__

/** @} */
