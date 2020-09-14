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

static uint16_t import_rsa_private_key(command_apdu_t *capdu)
{
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
