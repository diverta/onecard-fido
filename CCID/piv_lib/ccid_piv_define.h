/* 
 * File:   ccid_piv_define.h
 * Author: makmorit
 *
 * Created on 2023/01/03, 12:23
 */
#ifndef CCID_PIV_DEFINE_H
#define CCID_PIV_DEFINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 業務処理に関する定義
//
// コマンドバイト
#define PIV_INS_SELECT                  0xA4
#define PIV_INS_GET_DATA                0xCB
#define PIV_INS_VERIFY                  0x20
#define PIV_INS_CHANGE_REFERENCE_DATA   0x24
#define PIV_INS_RESET_RETRY_COUNTER     0x2C
#define PIV_INS_GENERAL_AUTHENTICATE    0x87
#define PIV_INS_PUT_DATA                0xDB

// 暗号化アルゴリズム
#define ALG_TDEA_3KEY       0x03
#define ALG_RSA_2048        0x07
#define ALG_ECC_256         0x11

// 3-key TDES関連
#define TDEA_BLOCK_SIZE     8

// tags for BER-TLV data object
#define TAG_WITNESS         0x80
#define TAG_CHALLENGE       0x81
#define TAG_RESPONSE        0x82
#define TAG_EXPONENT        0x85

#define LENGTH_CHALLENGE    16
#define LENGTH_AUTH_STATE   (5 + LENGTH_CHALLENGE)

// states for authenticate
#define AUTH_STATE_NONE     0
#define AUTH_STATE_EXTERNAL 1
#define AUTH_STATE_MUTUAL   2

//
// データオブジェクトのタグ
//
#define TAG_OBJ_CHUID       0x02
#define TAG_CERT_PAUTH      0x05
#define TAG_OBJ_CCC         0x07
#define TAG_CERT_DGSIG      0x0a
#define TAG_CERT_KEYMN      0x0b
#define TAG_KEY_PAUTH       0x9a
#define TAG_KEY_CAADM       0x9b
#define TAG_KEY_DGSIG       0x9c
#define TAG_KEY_KEYMN       0x9d
#define TAG_PIV_PIN         0x80
#define TAG_KEY_PUK         0x81

//
// データオブジェクト関連定義
//
#define CAADM_KEY_SIZE      24
#define ECC_PRV_KEY_SIZE    32
#define RSA2048_KEY_SIZE    640
#define RSA2048_N_LENGTH    256
#define RSA2048_PQ_LENGTH   128
#define MAX_CERT_SIZE       1024
#define MAX_CCC_SIZE        288
#define MAX_CHUID_SIZE      288

//
// デフォルトPIN番号・リトライカウンター
//
#define PUK_DEFAULT_CODE        "12345678"
#define PIN_DEFAULT_CODE        "123456\xFF\xFF"
#define PIN_DEFAULT_BUFFER_SIZE 8
#define PIN_DEFAULT_RETRY_CNT   3

//
// Yubico vendor specific instructions
//
#define YKPIV_INS_SET_MGMKEY            0xff
#define YKPIV_INS_IMPORT_ASYMMETRIC_KEY 0xfe
#define YKPIV_INS_GET_VERSION           0xfd
#define YKPIV_INS_RESET                 0xfb
#define YKPIV_INS_GET_SERIAL            0xf8

//
// BER-TLV関連
//  PIVのリクエスト／レスポンスに
//  格納されるデータオブジェクトは、
//  BER-TLV形式で表現されます
//
typedef struct {
    // Witness
    uint16_t wit_pos;
    int16_t  wit_len;
    // Challenge
    uint16_t chl_pos;
    int16_t  chl_len;
    // Response
    uint16_t rsp_pos;
    int16_t  rsp_len;
    // Exponentiation
    uint16_t exp_pos;
    int16_t  exp_len;
} BER_TLV_INFO;

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_DEFINE_H */
