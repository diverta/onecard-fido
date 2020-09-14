/* 
 * File:   ccid_ykpiv_import_key.c
 * Author: makmorit
 *
 * Created on 2020/09/14, 11:53
 */
#include <string.h>

#include "ccid.h"
#include "ccid_piv.h"
#include "ccid_piv_authenticate.h"
#include "ccid_piv_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for debug data
#define LOG_DEBUG_PKEY_BUFF     false

#define RSA2048_PQ_LENGTH 128

static uint16_t tlv_get_element_size(const uint8_t *data, const size_t size, size_t *elem_len_size, uint16_t *elem_size) 
{
    if (size < 1) {
        return SW_WRONG_LENGTH;

    } else if (data[0] < 0x80) {
        *elem_size = data[0];
        *elem_len_size = 1;

    } else if (data[0] == 0x81) {
        if (size < 2) {
            return SW_WRONG_LENGTH;

        } else {
            *elem_size = data[1];
            *elem_len_size = 2;
        }

    } else if (data[0] == 0x82) {
        if (size < 3) {
            return SW_WRONG_LENGTH;

        } else {
            *elem_size = (uint16_t)(data[1] << 8u) | data[2];
            *elem_len_size = 3;
        }

    } else {
        return SW_WRONG_LENGTH;
    }

    if (*elem_size + *elem_len_size > size) {
        // length does not overflow, but data does overflow
        return SW_WRONG_LENGTH;
    }

    if (*elem_size > RSA2048_PQ_LENGTH) {
        return SW_WRONG_DATA;
    }

    return SW_NO_ERROR;
}

static uint16_t import_rsa_private_key(command_apdu_t *capdu)
{
        if (capdu->lc == 0) {
            return SW_WRONG_LENGTH;
        }
        size_t   elem_len_size;
        uint16_t elem_size;
        uint16_t ret;

        // リクエストデータの格納領域
        uint8_t *cdata = capdu->data;
        uint8_t *p = cdata;
        //
        // Pの抽出
        //
        if (*p++ != 0x01) {
            return SW_WRONG_DATA;
        }
        ret = tlv_get_element_size(p, capdu->lc - 1, &elem_len_size, &elem_size);
        if (ret != SW_NO_ERROR) {
            return ret;
        }
        p += elem_len_size;
#if LOG_DEBUG_PKEY_BUFF
        uint8_t alg = capdu->p1;
        uint8_t tag = capdu->p2;
        fido_log_debug("ccid_ykpiv_ins_import_key: tag=%02x, alg=%02x", tag, alg);
        fido_log_debug("P (%d bytes) offset=%d:", elem_size, elem_len_size);
        fido_log_print_hexdump_debug(p, 64);
#endif
        p += elem_size;
        //
        // Qの抽出
        //
        if (*p++ != 0x02) {
            return SW_WRONG_DATA;
        }
        ret = tlv_get_element_size(p, capdu->lc - (p - cdata), &elem_len_size, &elem_size);
        if (ret != SW_NO_ERROR) {
            return ret;
        }
        p += elem_len_size;
#if LOG_DEBUG_PKEY_BUFF
        fido_log_debug("Q (%d bytes) offset=%d:", elem_size, elem_len_size);
        fido_log_print_hexdump_debug(p, 64);
#endif
        p += elem_size;
        //
        // DPの抽出
        //
        if (*p++ != 0x03) {
            return SW_WRONG_DATA;
        }
        ret = tlv_get_element_size(p, capdu->lc - (p - cdata), &elem_len_size, &elem_size);
        if (ret != SW_NO_ERROR) {
            return ret;
        }
        p += elem_len_size;
#if LOG_DEBUG_PKEY_BUFF
        fido_log_debug("DP (%d bytes) offset=%d:", elem_size, elem_len_size);
        fido_log_print_hexdump_debug(p, 64);
#endif
        p += elem_size;
        //
        // DQの抽出
        //
        if (*p++ != 0x04) {
            return SW_WRONG_DATA;
        }
        ret = tlv_get_element_size(p, capdu->lc - (p - cdata), &elem_len_size, &elem_size);
        if (ret != SW_NO_ERROR) {
            return ret;
        }
        p += elem_len_size;
#if LOG_DEBUG_PKEY_BUFF
        fido_log_debug("DQ (%d bytes) offset=%d:", elem_size, elem_len_size);
        fido_log_print_hexdump_debug(p, 64);
#endif
        p += elem_size;
        //
        // QINVの抽出
        //
        if (*p++ != 0x05) {
            return SW_WRONG_DATA;
        }
        ret = tlv_get_element_size(p, capdu->lc - (p - cdata), &elem_len_size, &elem_size);
        if (ret != SW_NO_ERROR) {
            return ret;
        }
        p += elem_len_size;
#if LOG_DEBUG_PKEY_BUFF
        fido_log_debug("QINV (%d bytes) offset=%d:", elem_size, elem_len_size);
        fido_log_print_hexdump_debug(p, 64);
#endif

    // 後ほど正式に実装予定
    return SW_WRONG_P1P2;
}

static uint16_t import_ecc_private_key(command_apdu_t *capdu)
{
    size_t priv_key_size = 32;
    if (capdu->lc < 2 + priv_key_size) {
        return SW_WRONG_LENGTH;
    }

    // リクエストデータの格納領域
    uint8_t *cdata = capdu->data;
    if (cdata[0] != 0x06 || cdata[1] != priv_key_size) {
        return SW_WRONG_DATA;
    }

#if LOG_DEBUG_PKEY_BUFF
    uint8_t alg = capdu->p1;
    uint8_t tag = capdu->p2;
    uint8_t *key = cdata + 2;
    fido_log_debug("ccid_ykpiv_ins_import_key: tag=%02x, alg=%02x (%d bytes)", tag, alg, priv_key_size);
    fido_log_print_hexdump_debug(key, priv_key_size);
#endif

    // 後ほど正式に実装予定
    return SW_WRONG_P1P2;
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
