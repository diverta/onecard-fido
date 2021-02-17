/* 
 * File:   ccid_openpgp_key.c
 * Author: makmorit
 *
 * Created on 2021/02/09, 16:20
 */
#include <string.h>

#include "ccid_openpgp.h"
#include "ccid_openpgp_key.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// テスト用
#define LOG_DEBUG_KEY_ATTR_DESC     false

//
// Keys for OpenPGP
//
#define KEY_TYPE_RSA                0x01

// 鍵ステータス種別
#define KEY_NOT_PRESENT             0x00
#define KEY_GENERATED               0x01
#define KEY_IMPORTED                0x02

//
// offset
//  1: Reserved for length of modulus, default: 2048
//  3: length of exponent: 32 bit
//
static uint8_t rsa_attr[] = {KEY_TYPE_RSA, 0x08, 0x00, 0x00, 0x20, 0x00};
static uint8_t key_fingerprint[KEY_FINGERPRINT_LENGTH];
static uint8_t key_datetime[KEY_DATETIME_LENGTH];

#ifdef OPENPGP_TEST_DATA
// Signature key
//   884B 9142 F645 1A72 4B92  EB94 DF80 CCEF FF19 F200
//   2021-01-01 01:38:34 GMT (1609465114)
//   key generated
// Encryption key
//   31C1 2190 FCF1 A684 5AF9  D719 26D7 28A8 F090 33F1
//   2021-01-01 01:38:44 GMT (1609465124)
//   key generated
// Authentication key
//   811F C45F 911A C15A F6DC  5BD6 58BA B8D1 3239 D981
//   2021-01-01 01:38:54 GMT (1609465134)
//   key generated
static uint8_t key_sig_fingerprint[] = {0x88, 0x4B, 0x91, 0x42, 0xF6, 0x45, 0x1A, 0x72, 0x4B, 0x92, 0xEB, 0x94, 0xDF, 0x80, 0xCC, 0xEF, 0xFF, 0x19, 0xF2, 0x00};
static uint8_t key_dec_fingerprint[] = {0x31, 0xC1, 0x21, 0x90, 0xFC, 0xF1, 0xA6, 0x84, 0x5A, 0xF9, 0xD7, 0x19, 0x26, 0xD7, 0x28, 0xA8, 0xF0, 0x90, 0x33, 0xF1};
static uint8_t key_aut_fingerprint[] = {0x81, 0x1F, 0xC4, 0x5F, 0x91, 0x1A, 0xC1, 0x5A, 0xF6, 0xDC, 0x5B, 0xD6, 0x58, 0xBA, 0xB8, 0xD1, 0x32, 0x39, 0xD9, 0x81};
static uint8_t key_sig_datetime[] = {0x5F, 0xEE, 0x7D, 0x1A};
static uint8_t key_dec_datetime[] = {0x5F, 0xEE, 0x7D, 0x24};
static uint8_t key_aut_datetime[] = {0x5F, 0xEE, 0x7D, 0x2E};
#endif

//
// 鍵属性管理
//
uint16_t openpgp_key_get_attributes(uint16_t tag, uint8_t *buf, size_t *size) 
{
    bool is_exist = false;
    size_t buffer_size;
    if (ccid_flash_object_read_by_tag(APPLET_OPENPGP, tag, &is_exist, buf, &buffer_size) == false) {
        // 読出しが失敗した場合はエラー
        fido_log_error("OpenPGP key attribute read fail: tag=0x%04x", tag);
        return SW_UNABLE_TO_PROCESS;
    }
    if (is_exist == false) {
        // Flash ROMに登録されていない場合はデフォルト（RSA-2048）を設定
        buffer_size = sizeof(rsa_attr);
        memcpy(buf, rsa_attr, buffer_size);
#if LOG_DEBUG_KEY_ATTR_DESC
        fido_log_debug("OpenPGP key attribute is not registered, use default(RSA-2048): tag=0x%04x", tag);
#endif
    }

    // サイズを戻す
    if (size != NULL) {
        *size = buffer_size;
    }

    // 正常終了
    return SW_NO_ERROR;
}

//
// 鍵フィンガープリント管理
//
uint16_t openpgp_key_get_fingerprint(uint16_t tag, void *buf, size_t *size)
{
    // TODO: 仮の実装です。
    switch (tag) {
        case TAG_KEY_SIG_FINGERPRINT:
            memcpy(buf, key_sig_fingerprint, sizeof(key_fingerprint));
            break;
        case TAG_KEY_DEC_FINGERPRINT:
            memcpy(buf, key_dec_fingerprint, sizeof(key_fingerprint));
            break;
        case TAG_KEY_AUT_FINGERPRINT:
            memcpy(buf, key_aut_fingerprint, sizeof(key_fingerprint));
            break;
        default:
            memset(buf, 0, sizeof(key_fingerprint));
            break;
    }
    *size = sizeof(key_fingerprint);
    
    // 正常終了
    return SW_NO_ERROR;
}

//
// 鍵タイムスタンプ管理
//
uint16_t openpgp_key_get_datetime(uint16_t tag, void *buf, size_t *size)
{
    // TODO: 仮の実装です。
    switch (tag) {
        case TAG_KEY_SIG_GENERATION_DATES:
            memcpy(buf, key_sig_datetime, sizeof(key_datetime));
            break;
        case TAG_KEY_DEC_GENERATION_DATES:
            memcpy(buf, key_dec_datetime, sizeof(key_datetime));
            break;
        case TAG_KEY_AUT_GENERATION_DATES:
            memcpy(buf, key_aut_datetime, sizeof(key_datetime));
            break;
        default:
            memset(buf, 0, sizeof(key_datetime));
            break;
    }
    *size = sizeof(key_datetime);

    // 正常終了
    return SW_NO_ERROR;
}

//
// 鍵ステータス管理
//
static uint16_t get_key_status_tag(OPGP_KEY_TYPE type) 
{
    switch (type) {
        case OPGP_KEY_SIG:
            return TAG_KEY_SIG_STATUS;
        case OPGP_KEY_ENC:
            return TAG_KEY_DEC_STATUS;
        case OPGP_KEY_AUT:
            return TAG_KEY_AUT_STATUS;
        default:
            return TAG_OPGP_NONE;
    }
}

uint16_t openpgp_key_get_status(OPGP_KEY_TYPE type, uint8_t *status)
{
    // データオブジェクトのタグを取得
    uint16_t tag = get_key_status_tag(type);
    if (tag == TAG_OPGP_NONE) {
        return SW_WRONG_DATA;
    }

    // 鍵ステータスをFlash ROMから読み込み
    bool is_exist = false;
    uint8_t status_;
    if (ccid_flash_object_read_by_tag(APPLET_OPENPGP, tag, &is_exist, &status_, NULL) == false) {
        // 読出しが失敗した場合はエラー
        fido_log_error("OpenPGP key status read fail: tag=0x%04x", tag);
        return SW_UNABLE_TO_PROCESS;
    }
    if (is_exist == false) {
        // Flash ROMに登録されていない場合はデフォルトを設定
        status_ = KEY_NOT_PRESENT;
    }

    // ステータスを戻す
    if (status != NULL) {
        *status = status_;
    }

    // 正常終了
    return SW_NO_ERROR;
}
