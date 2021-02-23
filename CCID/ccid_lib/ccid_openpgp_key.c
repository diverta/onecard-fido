/* 
 * File:   ccid_openpgp_key.c
 * Author: makmorit
 *
 * Created on 2021/02/09, 16:20
 */
#include <string.h>

#include "ccid_openpgp.h"
#include "ccid_openpgp_data.h"
#include "ccid_openpgp_key.h"
#include "ccid_openpgp_key_rsa.h"

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
    bool is_exist = false;
    size_t buffer_size;
    if (ccid_flash_object_read_by_tag(APPLET_OPENPGP, tag, &is_exist, buf, &buffer_size) == false) {
        // 読出しが失敗した場合はエラー
        fido_log_error("OpenPGP key fingerprint read fail: tag=0x%04x", tag);
        return SW_UNABLE_TO_PROCESS;
    }
    if (is_exist == false) {
        // Flash ROMに登録されていない場合はデフォルトを設定
        buffer_size = KEY_FINGERPRINT_LENGTH;
        memset(buf, 0, KEY_FINGERPRINT_LENGTH);
    }

    // サイズを戻す
    if (size != NULL) {
        *size = buffer_size;
    }
    
    // 正常終了
    return SW_NO_ERROR;
}

//
// 鍵タイムスタンプ管理
//
uint16_t openpgp_key_get_datetime(uint16_t tag, void *buf, size_t *size)
{
    bool is_exist = false;
    size_t buffer_size;
    if (ccid_flash_object_read_by_tag(APPLET_OPENPGP, tag, &is_exist, buf, &buffer_size) == false) {
        // 読出しが失敗した場合はエラー
        fido_log_error("OpenPGP key generate datetime read fail: tag=0x%04x", tag);
        return SW_UNABLE_TO_PROCESS;
    }
    if (is_exist == false) {
        // Flash ROMに登録されていない場合はデフォルトを設定
        buffer_size = KEY_DATETIME_LENGTH;
        memset(buf, 0, KEY_DATETIME_LENGTH);
    }

    // サイズを戻す
    if (size != NULL) {
        *size = buffer_size;
    }

    // 正常終了
    return SW_NO_ERROR;
}

//
// 鍵ステータス管理
//
uint16_t ccid_openpgp_key_status_tag_get(uint16_t key_tag) 
{
    switch (key_tag) {
        case TAG_KEY_SIG:
            return TAG_KEY_SIG_STATUS;
        case TAG_KEY_DEC:
            return TAG_KEY_DEC_STATUS;
        case TAG_KEY_AUT:
            return TAG_KEY_AUT_STATUS;
        default:
            return TAG_OPGP_NONE;
    }
}

uint16_t openpgp_key_get_status(uint16_t key_tag, uint8_t *status)
{
    // データオブジェクトのタグを取得
    uint16_t tag = ccid_openpgp_key_status_tag_get(key_tag);
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

uint16_t ccid_openpgp_key_is_present(uint16_t key_tag)
{
    // 鍵ステータスを取得
    uint8_t status;
    uint16_t sw = openpgp_key_get_status(key_tag, &status);
    if (sw != SW_NO_ERROR) {
        return sw;
    }
    // If key not present
    if (status == KEY_NOT_PRESENT) {
        return SW_REFERENCE_DATA_NOT_FOUND;
    } else {
        return SW_NO_ERROR;
    }    
}

//
// 鍵生成処理
//
static uint8_t m_key_attr[16];

static uint16_t get_key_tag(uint8_t tag)
{
    switch (tag) {
        case 0xB6:
            return TAG_KEY_SIG;
        case 0xB8:
            return TAG_KEY_DEC;
        case 0xA4:
            return TAG_KEY_AUT;
        default:
            return TAG_OPGP_NONE;
    }
}

static uint16_t get_key_attribute_tag(uint16_t key_tag) 
{
    switch (key_tag) {
        case TAG_KEY_SIG:
            return TAG_ALGORITHM_ATTRIBUTES_SIG;
        case TAG_KEY_DEC:
            return TAG_ALGORITHM_ATTRIBUTES_DEC;
        case TAG_KEY_AUT:
            return TAG_ALGORITHM_ATTRIBUTES_AUT;
        default:
            return TAG_OPGP_NONE;
    }
}

static uint16_t get_key_attribute(uint16_t key_tag, uint8_t *key_attr_buf, size_t *key_attr_size)
{
    // 鍵種別から、データオブジェクトのタグを取得
    uint16_t key_attr_tag = get_key_attribute_tag(key_tag);
    if (key_attr_tag == TAG_OPGP_NONE) {
        return SW_WRONG_DATA;
    }

    // 鍵属性を取得
    uint16_t sw = openpgp_key_get_attributes(key_attr_tag, key_attr_buf, key_attr_size);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

#if LOG_DEBUG_KEY_ATTR_DESC
    fido_log_debug("OpenPGP key attribute buffer (tag=0x%04x): ", key_attr_tag);
    fido_log_print_hexdump_debug(key_attr_buf, *key_attr_size);
#endif

    // 正常終了
    return SW_NO_ERROR;
}

static void key_pair_generate_response(response_apdu_t *rapdu, uint8_t *key_attr)
{
    uint8_t *rdata = rapdu->data;
    rdata[0] = 0x7f;
    rdata[1] = 0x49;
    if (key_attr[0] == KEY_TYPE_RSA) {
        uint16_t nbits = (key_attr[1] << 8) | key_attr[2];
        uint16_t n_size = nbits / 8;
        uint8_t e_size = ccid_crypto_rsa_e_size();
        rdata[2] = 0x82;
        rdata[3] = HI(6 + n_size + e_size);
        rdata[4] = LO(6 + n_size + e_size);
        rdata[5] = 0x81; 
        // modulus
        rdata[6] = 0x82;
        rdata[7] = HI(n_size);
        rdata[8] = LO(n_size);
        memcpy(rdata + 9, ccid_openpgp_key_rsa_public_key(), n_size);
        // exponent
        rdata[9 + n_size] = 0x82; 
        rdata[10 + n_size] = e_size;
        memcpy(rdata + 11 + n_size, ccid_crypto_rsa_e_bytes(), e_size);
        rapdu->len = 11 + n_size + e_size;
    } else {
        rapdu->len = 0;
    }
}

uint16_t ccid_openpgp_key_pair_generate(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }
    if (capdu->lc != 2 && capdu->lc != 5) {
        return SW_WRONG_LENGTH;
    }

    // 引数のタグから鍵種別を取得
    uint16_t key_tag = get_key_tag(capdu->data[0]);
    if (key_tag == TAG_OPGP_NONE) {
        return SW_WRONG_DATA;
    }

    // 鍵属性を取得
    size_t key_attr_size;
    uint16_t sw = get_key_attribute(key_tag, m_key_attr, &key_attr_size);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // 鍵アルゴリズムを取得
    uint8_t key_alg = m_key_attr[0];
    if (key_alg != KEY_TYPE_RSA) {
        // RSA-2048以外はサポート外
        fido_log_error("OpenPGP do not support algorithm 0x%02x ", key_alg);
        return SW_WRONG_DATA;
    }

    if (capdu->p1 == 0x80) {
        // RSA-2048キーペアを生成
        sw = ccid_openpgp_key_rsa_generate(m_key_attr);
        if (sw != SW_NO_ERROR) {
            return sw;
        }
        // レスポンスを生成
        key_pair_generate_response(rapdu, m_key_attr);
        // Flash ROMに公開鍵を保存
        sw = ccid_openpgp_data_register_key(capdu, rapdu, key_tag, KEY_GENERATED);
        if (sw != SW_NO_ERROR) {
            return sw;
        }

    } else if (capdu->p1 == 0x81) {
        // 鍵ステータスを参照し、鍵がない場合はエラー
        sw = ccid_openpgp_key_is_present(TAG_KEY_SIG);
        if (sw != SW_NO_ERROR) {
            return sw;
        }
        // Flash ROMから公開鍵を取得
        sw = ccid_openpgp_key_rsa_read(key_tag);
        if (sw != SW_NO_ERROR) {
            return sw;
        }
        // レスポンスを生成
        key_pair_generate_response(rapdu, m_key_attr);

    } else {
        return SW_WRONG_P1P2;
    }
    
    // 正常終了
    return SW_NO_ERROR;
}
