/* 
 * File:   ccid_piv_object.c
 * Author: makmorit
 *
 * Created on 2020/06/02, 11:06
 */
#include <stdlib.h>
#include <string.h>

#include "ccid_piv.h"
#include "ccid_piv_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// テスト用
#include "ccid_piv_object_test.h"

// デフォルトの管理用キー (24 bytes)
static char *card_admin_key_default = "010203040506070801020304050607080102030405060708";

static size_t convert_hexstring_to_bytes(char *data, uint8_t *buffer)
{
    char buf[8];
    char *e;
    unsigned long value;
    size_t index = 0;
    size_t length = strlen(data);
    memset(buf, 0, sizeof(buf));

    for (int i = 0; i < length; i += 2) {
        // Hex文字列を２文字分抽出
        strncpy(buf, data + i, 2);
        // Hex文字列を数値に変換する
        value = strtoul(buf, &e, 16);
        if (*e != 0) { 
            // 変換失敗した場合は処理終了
            return 0;

        } else {
            buffer[index++] = (uint8_t)value;
        }
    }
    return index;
}

bool ccid_piv_object_sn_get(uint8_t *buffer, size_t *size)
{
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(SN_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("Serial number is requested (%d bytes)", *size);
    return false;
#endif
}

bool ccid_piv_object_chuid_get(uint8_t *buffer, size_t *size)
{
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(CHUID_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("Card Holder Unique Identifier is requested (%d bytes)", *size);
    return false;
#endif
}

bool ccid_piv_object_ccc_get(uint8_t *buffer, size_t *size)
{
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(CCC_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("Card Capability Container is requested");
    return false;
#endif
}

bool ccid_piv_object_cert_cauth_get(uint8_t *buffer, size_t *size)
{
    // 後日正式に実装予定です。
    fido_log_debug("X.509 Certificate for Card Authentication is requested");
    return false;
}

bool ccid_piv_object_cert_pauth_get(uint8_t *buffer, size_t *size)
{
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(TAG05_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("X.509 Certificate for PIV Authentication is requested (%d bytes)", *size);
    return false;
#endif
}

bool ccid_piv_object_cert_digsig_get(uint8_t *buffer, size_t *size)
{
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(TAG0A_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("X.509 Certificate for Digital Signature is requested (%d bytes)", *size);
    return false;
#endif
}

bool ccid_piv_object_cert_keyman_get(uint8_t *buffer, size_t *size)
{
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(TAG0B_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("X.509 Certificate for Key Management is requested (%d bytes)", *size);
    return false;
#endif
}

bool ccid_piv_object_key_history_get(uint8_t *buffer, size_t *size)
{
    // 後日正式に実装予定です。
    fido_log_debug("Key History Object is requested");
    return false;
}

bool ccid_piv_object_card_admin_key_get(uint8_t *buffer, size_t *size)
{
    // デフォルトを戻す
    *size = convert_hexstring_to_bytes(card_admin_key_default, buffer);
    fido_log_debug("Card administration key is requested (%d bytes)", *size);
    return true;
}

uint8_t ccid_piv_object_card_admin_key_alg_get(void)
{
    // デフォルトを戻す
    // 0x03: 3-key triple DEA
    // 0x07: ALG_RSA_2048
    // 0x11: ALG_ECC_256
    return 0x03;
}
