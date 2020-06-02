/* 
 * File:   ccid_piv_object.c
 * Author: makmorit
 *
 * Created on 2020/06/02, 11:06
 */
#include <stdlib.h>
#include <string.h>

#include "ccid_piv_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// テスト用の仮データ
//  正式には、yubikey-piv-tool を利用し、
//  Flash ROMにインストールした証明書等データを
//  使用することになります。
//
static char *chuid_temp = "533B3019D4E739DA739CED39CE739D836858210842108421C84210C3EB34104C8D536A86AA98A5CE20D53557776E58350832303330303130313E00FE00";

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

bool ccid_piv_object_chuid_get(uint8_t *buffer, size_t *size)
{
    // 仮の実装です。
    *size = convert_hexstring_to_bytes(chuid_temp, buffer);
    fido_log_debug("Card Holder Unique Identifier is requested (%d bytes)", *size);
    return true;
}

bool ccid_piv_object_ccc_get(uint8_t *buffer, size_t *size)
{
    // 仮の実装です。
    fido_log_debug("Card Capability Container is requested");
    return false;
}

bool ccid_piv_object_cert_cauth_get(uint8_t *buffer, size_t *size)
{
    // 仮の実装です。
    fido_log_debug("X.509 Certificate for Card Authentication is requested");
    return false;
}

bool ccid_piv_object_cert_pauth_get(uint8_t *buffer, size_t *size)
{
    // 仮の実装です。
    fido_log_debug("X.509 Certificate for PIV Authentication is requested");
    return false;
}

bool ccid_piv_object_cert_digsig_get(uint8_t *buffer, size_t *size)
{
    // 仮の実装です。
    fido_log_debug("X.509 Certificate for Digital Signature is requested");
    return false;
}

bool ccid_piv_object_cert_keyman_get(uint8_t *buffer, size_t *size)
{
    // 仮の実装です。
    fido_log_debug("X.509 Certificate for Key Management is requested");
    return false;
}

bool ccid_piv_object_key_history_get(uint8_t *buffer, size_t *size)
{
    // 仮の実装です。
    fido_log_debug("Key History Object is requested");
    return false;
}
