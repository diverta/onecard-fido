/* 
 * File:   ccid_openpgp_define.h
 * Author: makmorit
 *
 * Created on 2023/01/03, 13:54
 */
#ifndef CCID_OPENPGP_DEFINE_H
#define CCID_OPENPGP_DEFINE_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 業務処理に関する定義
//
// コマンドバイト
#define OPENPGP_INS_VERIFY                  0x20
#define OPENPGP_INS_CHANGE_REFERENCE_DATA   0x24
#define OPENPGP_INS_PSO                     0x2A
#define OPENPGP_INS_RESET_RETRY_COUNTER     0x2C
#define OPENPGP_INS_GENERATE_ASYMM_KEY_PAIR 0x47
#define OPENPGP_INS_ACTIVATE                0x44
#define OPENPGP_INS_SELECT                  0xA4
#define OPENPGP_INS_GET_DATA                0xCA
#define OPENPGP_INS_PUT_DATA                0xDA
#define OPENPGP_INS_IMPORT_KEY              0xDB
#define OPENPGP_INS_TERMINATE               0xE6

//
// データオブジェクトのタグ
//   以下はOpenPGPの仕様に存在しない定義
#define TAG_OPGP_NONE                       0x00
#define TAG_OPGP_PW1                        0x01
#define TAG_OPGP_PW3                        0x02
#define TAG_OPGP_RC                         0x03
#define TAG_KEY_SIG_STATUS                  0x04
#define TAG_KEY_DEC_STATUS                  0x05
#define TAG_KEY_AUT_STATUS                  0x06
#define TAG_KEY_SIG                         0x07
#define TAG_KEY_DEC                         0x08
#define TAG_KEY_AUT                         0x09
#define TAG_ATTR_TERMINATED                 0xFC
//   以下はOpenPGPの仕様に存在する定義
#define TAG_AID                             0x4F
#define TAG_NAME                            0x5B
#define TAG_LOGIN                           0x5E
#define TAG_LANG                            0x5F2D
#define TAG_SEX                             0x5F35
#define TAG_URL                             0x5F50
#define TAG_HISTORICAL_BYTES                0x5F52
#define TAG_CARDHOLDER_RELATED_DATA         0x65
#define TAG_APPLICATION_RELATED_DATA        0x6E
#define TAG_DISCRETIONARY_DATA_OBJECTS      0x73
#define TAG_SECURITY_SUPPORT_TEMPLATE       0x7A
#define TAG_EXTENDED_LENGTH_INFO            0x7F66
#define TAG_DIGITAL_SIG_COUNTER             0x93
#define TAG_EXTENDED_CAPABILITIES           0xC0
#define TAG_ALGORITHM_ATTRIBUTES_SIG        0xC1
#define TAG_ALGORITHM_ATTRIBUTES_DEC        0xC2
#define TAG_ALGORITHM_ATTRIBUTES_AUT        0xC3
#define TAG_PW_STATUS                       0xC4
#define TAG_KEY_FINGERPRINTS                0xC5
#define TAG_CA_FINGERPRINTS                 0xC6
#define TAG_KEY_SIG_FINGERPRINT             0xC7
#define TAG_KEY_DEC_FINGERPRINT             0xC8
#define TAG_KEY_AUT_FINGERPRINT             0xC9
#define TAG_KEY_CA1_FINGERPRINT             0xCA
#define TAG_KEY_CA2_FINGERPRINT             0xCB
#define TAG_KEY_CA3_FINGERPRINT             0xCC
#define TAG_KEY_GENERATION_DATES            0xCD
#define TAG_KEY_SIG_GENERATION_DATES        0xCE
#define TAG_KEY_DEC_GENERATION_DATES        0xCF
#define TAG_KEY_AUT_GENERATION_DATES        0xD0
#define TAG_RESETTING_CODE                  0xD3
#define TAG_KEY_INFO                        0xDE

//
// Keys for OpenPGP
//
#define OPGP_KEY_TYPE_RSA                   0x01

// 鍵ステータス種別
#define OPGP_KEY_NOT_PRESENT                0x00
#define OPGP_KEY_IMPORTED                   0x02

#define OPGP_MAX_CERT_LENGTH                0x480
#define OPGP_MAX_DO_LENGTH                  0xFF

#define OPGP_KEY_FINGERPRINT_LENGTH         20
#define OPGP_KEY_DATETIME_LENGTH            4

//
// PIN番号・署名カウンター関連
//
#define OPGP_MAX_PIN_LENGTH                 64
#define OPGP_DIGITAL_SIG_COUNTER_LENGTH     3

#define OPGP_PIN_PW1_CODE_DEFAULT           "123456"
#define OPGP_PIN_PW3_CODE_DEFAULT           "12345678"
#define OPGP_PIN_RC_CODE_DEFAULT            "12345678"

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_DEFINE_H */
