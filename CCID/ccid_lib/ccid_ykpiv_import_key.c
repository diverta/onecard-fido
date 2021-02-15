/* 
 * File:   ccid_ykpiv_import_key.c
 * Author: makmorit
 *
 * Created on 2020/09/14, 11:53
 */
#include <string.h>

#include "ccid.h"
#include "ccid_flash_object.h"
#include "ccid_piv.h"
#include "ccid_piv_authenticate.h"
#include "ccid_piv_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for debug data
#define LOG_DEBUG_PKEY_BUFF     false

static uint16_t tlv_get_element_size(uint8_t elem_no, uint8_t *data, size_t size, size_t *elem_header_size, uint16_t *elem_data_size, uint16_t elem_data_size_max) 
{
    if (size < 1) {
        return SW_WRONG_LENGTH;
    }
    if (data[0] != elem_no) {
        return SW_WRONG_DATA;
    }
    
    if (data[1] < 0x80) {
        *elem_data_size = data[1];
        *elem_header_size = 2;

    } else if (data[1] == 0x81) {
        if (size < 2) {
            return SW_WRONG_LENGTH;

        } else {
            *elem_data_size = data[2];
            *elem_header_size = 3;
        }

    } else if (data[1] == 0x82) {
        if (size < 3) {
            return SW_WRONG_LENGTH;

        } else {
            *elem_data_size = (uint16_t)(data[2] << 8u) | data[3];
            *elem_header_size = 4;
        }

    } else {
        return SW_WRONG_LENGTH;
    }

    if (*elem_data_size + *elem_header_size > size) {
        // length does not overflow, but data does overflow
        return SW_WRONG_LENGTH;
    }

    if (*elem_data_size > elem_data_size_max) {
        return SW_WRONG_DATA;
    }

    return SW_NO_ERROR;
}

static uint16_t import_rsa_private_key(command_apdu_t *capdu)
{
    if (capdu->lc == 0) {
        return SW_WRONG_LENGTH;
    }
    size_t   elem_header_size;
    uint16_t elem_data_size;
    uint16_t ret;

    // リクエストデータの格納領域
    uint8_t *cdata = capdu->data;
    uint8_t *p = cdata;

    //
    // Pの抽出
    //
    size_t remaining = capdu->lc;
    ret = tlv_get_element_size(0x01, p, remaining, &elem_header_size, &elem_data_size, RSA2048_PQ_LENGTH);
    if (ret != SW_NO_ERROR) {
        return ret;
    }
    // 鍵データの先頭アドレスを保持
    p += elem_header_size;
    uint8_t *P = p;
    // 次の項目に移動
    p += elem_data_size;

    //
    // Qの抽出
    //
    remaining = capdu->lc - (p - cdata);
    ret = tlv_get_element_size(0x02, p, remaining, &elem_header_size, &elem_data_size, RSA2048_PQ_LENGTH);
    if (ret != SW_NO_ERROR) {
        return ret;
    }
    // 鍵データの先頭アドレスを保持
    p += elem_header_size;
    uint8_t *Q = p;
    // 次の項目に移動
    p += elem_data_size;

    //
    // DPの抽出
    //
    remaining = capdu->lc - (p - cdata);
    ret = tlv_get_element_size(0x03, p, remaining, &elem_header_size, &elem_data_size, RSA2048_PQ_LENGTH);
    if (ret != SW_NO_ERROR) {
        return ret;
    }
    // 鍵データの先頭アドレスを保持
    p += elem_header_size;
    uint8_t *DP = p;
    // 次の項目に移動
    p += elem_data_size;

    //
    // DQの抽出
    //
    remaining = capdu->lc - (p - cdata);
    ret = tlv_get_element_size(0x04, p, remaining, &elem_header_size, &elem_data_size, RSA2048_PQ_LENGTH);
    if (ret != SW_NO_ERROR) {
        return ret;
    }
    // 鍵データの先頭アドレスを保持
    p += elem_header_size;
    uint8_t *DQ = p;
    // 次の項目に移動
    p += elem_data_size;

    //
    // QINVの抽出
    //
    remaining = capdu->lc - (p - cdata);
    ret = tlv_get_element_size(0x05, p, remaining, &elem_header_size, &elem_data_size, RSA2048_PQ_LENGTH);
    if (ret != SW_NO_ERROR) {
        return ret;
    }
    // 鍵データの先頭アドレスを保持
    p += elem_header_size;
    uint8_t *QINV = p;

    // 鍵データを、Flash ROM書出用バッファにコピー（９バイト目を先頭とする）
    uint8_t *key_data_buff = ccid_flash_object_write_buffer() + 8;
    size_t key_size = 0;
    memcpy(key_data_buff + key_size, P, RSA2048_PQ_LENGTH);
    key_size += RSA2048_PQ_LENGTH;
    memcpy(key_data_buff + key_size, Q, RSA2048_PQ_LENGTH);
    key_size += RSA2048_PQ_LENGTH;
    memcpy(key_data_buff + key_size, DP, RSA2048_PQ_LENGTH);
    key_size += RSA2048_PQ_LENGTH;
    memcpy(key_data_buff + key_size, DQ, RSA2048_PQ_LENGTH);
    key_size += RSA2048_PQ_LENGTH;
    memcpy(key_data_buff + key_size, QINV, RSA2048_PQ_LENGTH);
    key_size += RSA2048_PQ_LENGTH;

    // 秘密鍵を登録
    uint8_t key_alg = capdu->p1;
    uint8_t key_tag = capdu->p2;
    if (ccid_flash_piv_object_private_key_write(key_tag, key_alg, key_data_buff, key_size) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 正常終了
    return SW_NO_ERROR;
}

static uint16_t import_ecc_private_key(command_apdu_t *capdu)
{
    size_t priv_key_size = ECC_PRV_KEY_SIZE;
    if (capdu->lc < 2 + priv_key_size) {
        return SW_WRONG_LENGTH;
    }

    // リクエストデータの格納領域
    uint8_t *cdata = capdu->data;
    if (cdata[0] != 0x06 || cdata[1] != priv_key_size) {
        return SW_WRONG_DATA;
    }

    // 処理のパラメーターを取得
    uint8_t key_alg = capdu->p1;
    uint8_t key_tag = capdu->p2;
    uint8_t *key = cdata + 2;

    // 秘密鍵を登録
    if (ccid_flash_piv_object_private_key_write(key_tag, key_alg, key, priv_key_size) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 正常終了
    return SW_NO_ERROR;
}

uint16_t ccid_ykpiv_import_key(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    // 管理コマンドが実行可能でない場合は終了
    if (ccid_piv_admin_mode_get() == false) {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // パラメーターのチェック
    uint8_t alg = capdu->p1;
    uint8_t key_tag = capdu->p2;
    if (ccid_piv_object_is_key_tag_exist(key_tag) == false) {
        return SW_WRONG_P1P2;
    }

    // 秘密鍵を抽出
    uint16_t sw;
    switch (alg) {
        case ALG_RSA_2048:
            sw = import_rsa_private_key(capdu);
            break;
        case ALG_ECC_256:
            sw = import_ecc_private_key(capdu);
            break;
        default:
            sw = SW_WRONG_P1P2;
            break;
    }

    // 処理ステータスを戻す
    return sw;
}
