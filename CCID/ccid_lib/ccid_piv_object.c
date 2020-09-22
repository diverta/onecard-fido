/* 
 * File:   ccid_piv_object.c
 * Author: makmorit
 *
 * Created on 2020/06/02, 11:06
 */
#include <stdlib.h>
#include <string.h>

#include "ccid_piv.h"
#include "ccid_piv_authenticate.h"
#include "ccid_piv_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// テスト用
#define CCID_PIV_OBJECT_TEST false

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

static bool read_object_data(uint8_t tag, uint8_t *buffer, size_t *size)
{
    // オブジェクトデータをFlash ROMから読出し
    bool is_exist;
    if (ccid_flash_piv_object_data_read(tag, buffer, size, &is_exist) == false) {
        // 読出しが失敗した場合はエラー
        fido_log_error("PIV object read fail: tag=0x%02x", tag);
        return false;
    }

    if (is_exist == false) {
        // Flash ROMに登録されていない場合はエラー
        fido_log_error("PIV object is not registered: tag=0x%02x", tag);
        return false;
    }

    return true;
}

bool ccid_piv_object_chuid_get(uint8_t *buffer, size_t *size)
{
    // オブジェクトデータをFlash ROMから読出し
    if (read_object_data(TAG_OBJ_CHUID, buffer, size) == false) {
        return false;
    }

    fido_log_debug("Card Holder Unique Identifier is requested (%d bytes)", *size);
    return true;
}

bool ccid_piv_object_ccc_get(uint8_t *buffer, size_t *size)
{
    // オブジェクトデータをFlash ROMから読出し
    if (read_object_data(TAG_OBJ_CCC, buffer, size) == false) {
        return false;
    }

    fido_log_debug("Card Capability Container is requested (%d bytes)", *size);
    return true;
}

static bool read_private_key(uint8_t tag, uint8_t alg, uint8_t *buffer, size_t *size)
{
    // 秘密鍵データをFlash ROMから読出し
    bool is_exist;
    if (ccid_flash_piv_object_private_key_read(tag, alg, buffer, size, &is_exist) == false) {
        // 読出しが失敗した場合はエラー
        fido_log_error("Private Key for PIV application read fail: tag=0x%02x", tag);
        return false;
    }

    if (is_exist == false) {
        // Flash ROMに登録されていない場合はエラー
        fido_log_error("Private Key for PIV application is not registered: tag=0x%02x", tag);
        return false;
    }

    return true;
}

bool ccid_piv_object_key_pauth_get(uint8_t alg, uint8_t *buffer, size_t *size)
{
    // 秘密鍵をFlash ROMから読出し
    if (read_private_key(TAG_KEY_PAUTH, alg, buffer, size) == false) {
        return false;
    }

    fido_log_debug("Private Key for PIV Authentication is requested (%d bytes)", *size);
    return true;
}

bool ccid_piv_object_cert_pauth_get(uint8_t *buffer, size_t *size)
{
    // オブジェクトデータをFlash ROMから読出し
    if (read_object_data(TAG_CERT_PAUTH, buffer, size) == false) {
        return false;
    }

    fido_log_debug("X.509 Certificate for PIV Authentication is requested (%d bytes)", *size);
    return true;
}

bool ccid_piv_object_key_digsig_get(uint8_t alg, uint8_t *buffer, size_t *size)
{
    // 秘密鍵をFlash ROMから読出し
    if (read_private_key(TAG_KEY_DGSIG, alg, buffer, size) == false) {
        return false;
    }

    fido_log_debug("Private Key for Digital Signature is requested (%d bytes)", *size);
    return true;
}

bool ccid_piv_object_cert_digsig_get(uint8_t *buffer, size_t *size)
{
    // オブジェクトデータをFlash ROMから読出し
    if (read_object_data(TAG_CERT_DGSIG, buffer, size) == false) {
        return false;
    }

    fido_log_debug("X.509 Certificate for Digital Signature is requested (%d bytes)", *size);
    return true;
}

bool ccid_piv_object_key_keyman_get(uint8_t alg, uint8_t *buffer, size_t *size)
{
    // 秘密鍵をFlash ROMから読出し
    if (read_private_key(TAG_KEY_KEYMN, alg, buffer, size) == false) {
        return false;
    }

    fido_log_debug("Private Key for Key Management is requested (%d bytes)", *size);
    return true;
}

bool ccid_piv_object_cert_keyman_get(uint8_t *buffer, size_t *size)
{
    // オブジェクトデータをFlash ROMから読出し
    if (read_object_data(TAG_CERT_KEYMN, buffer, size) == false) {
        return false;
    }

    fido_log_debug("X.509 Certificate for Key Management is requested (%d bytes)", *size);
    return true;
}

bool ccid_piv_object_card_admin_key_get(uint8_t *buffer, size_t *size, uint8_t *alg)
{
    // パスワードをFlash ROMから読出し
    bool is_exist;
    if (ccid_flash_piv_object_card_admin_key_read(buffer, size, alg, &is_exist) == false) {
        // 読出しが失敗した場合はエラー
        fido_log_error("Card administration key read fail");
        return false;
    }

    if (is_exist) {
        fido_log_debug("Card administration key is requested (%d bytes)", *size);
    } else {
        // Flash ROMに登録されていない場合は
        // デフォルトを戻す
        *size = convert_hexstring_to_bytes(card_admin_key_default, buffer);
        *alg = ALG_TDEA_3KEY;
        fido_log_debug("Card administration key (default) is applied");
    }
    return true;
}

bool ccid_piv_object_get(uint8_t data_obj_tag, uint8_t *buffer, size_t *size)
{
    // パラメーターチェック
    if (*size < 1) {
        return false;
    }
    
    // PIVデータオブジェクトごとに処理を分岐
    bool success = false;
    switch (data_obj_tag) {
        case TAG_OBJ_CHUID:
            // Card Holder Unique Identifier
            success = ccid_piv_object_chuid_get(buffer, size);
            break;
        case TAG_CERT_PAUTH:
            // X.509 Certificate for PIV Authentication
            success = ccid_piv_object_cert_pauth_get(buffer, size);
            break;
        case TAG_OBJ_CCC:
            // Card Capability Container
            success = ccid_piv_object_ccc_get(buffer, size);
            break;
        case TAG_CERT_DGSIG:
            // X.509 Certificate for Digital Signature
            success = ccid_piv_object_cert_digsig_get(buffer, size);
            break;
        case TAG_CERT_KEYMN:
            // X.509 Certificate for Key Management
            success = ccid_piv_object_cert_keyman_get(buffer, size);
            break;
        default:
            break;
    }
    
    if (success == false) {
        // 処理失敗時は長さをゼロクリア
        *size = 0;
    }
    
    // 正常終了
    return success;
}

bool ccid_piv_object_is_key_tag_exist(uint8_t key_tag)
{
    switch (key_tag) {
        case TAG_KEY_PAUTH:
        case TAG_KEY_CAADM:
        case TAG_KEY_DGSIG:
        case TAG_KEY_KEYMN:
            return true;
        default:
            return false;
    }
}

bool ccid_piv_object_is_obj_tag_exist(uint8_t obj_tag)
{
    switch (obj_tag) {
        case TAG_OBJ_CHUID:
        case TAG_CERT_PAUTH:
        case TAG_OBJ_CCC:
        case TAG_CERT_DGSIG:
        case TAG_CERT_KEYMN:
            return true;
        default:
            return false;
    }
}

#if CCID_PIV_OBJECT_TEST
//
// 以下はテスト用
//
static uint8_t work_buf[1024];

void ccid_piv_object_test_read_ecc_pkey(void)
{
    size_t s;
    if (ccid_piv_object_key_pauth_get(ALG_ECC_256, work_buf, &s)) {
        fido_log_debug("ccid_piv_object_key_pauth_get (%d bytes)", s);
        fido_log_print_hexdump_debug(work_buf, s);
    }
    if (ccid_piv_object_key_digsig_get(ALG_ECC_256, work_buf, &s)) {
        fido_log_debug("ccid_piv_object_key_digsig_get (%d bytes)", s);
        fido_log_print_hexdump_debug(work_buf, s);
    }
    if (ccid_piv_object_key_keyman_get(ALG_ECC_256, work_buf, &s)) {
        fido_log_debug("ccid_piv_object_key_keyman_get (%d bytes)", s);
        fido_log_print_hexdump_debug(work_buf, s);
    }
}

void ccid_piv_object_test_read_rsa_pkey(void)
{
    // for research
    size_t s;
    static uint8_t cnt=0;
    switch (cnt) {
        case 0:
            fido_log_debug("Private key read test start.");
            ccid_piv_object_key_pauth_get(ALG_RSA_2048, work_buf, &s);
            cnt++;
            break;
        case 1:
            ccid_piv_object_key_digsig_get(ALG_RSA_2048, work_buf, &s);
            cnt++;
            break;
        case 2:
            ccid_piv_object_key_keyman_get(ALG_RSA_2048, work_buf, &s);
            cnt++;
            break;
        default:
            fido_log_debug("Private key read test end.");
            cnt=0;
            return;
    }

    uint8_t *p = work_buf;
    fido_log_debug("P (first 16 bytes):", s);
    fido_log_print_hexdump_debug(p, 16);
    fido_log_debug("P (last 16 bytes):", s);
    fido_log_print_hexdump_debug(p+128-16, 16);
    p += 128;
    fido_log_debug("Q (first 16 bytes):", s);
    fido_log_print_hexdump_debug(p, 16);
    fido_log_debug("Q (last 16 bytes):", s);
    fido_log_print_hexdump_debug(p+128-16, 16);
    p += 128;
    fido_log_debug("DP (first 16 bytes):", s);
    fido_log_print_hexdump_debug(p, 16);
    fido_log_debug("DP (last 16 bytes):", s);
    fido_log_print_hexdump_debug(p+128-16, 16);
    p += 128;
    fido_log_debug("DQ (first 16 bytes):", s);
    fido_log_print_hexdump_debug(p, 16);
    fido_log_debug("DQ (last 16 bytes):", s);
    fido_log_print_hexdump_debug(p+128-16, 16);
    p += 128;
    fido_log_debug("QINV (first 16 bytes):", s);
    fido_log_print_hexdump_debug(p, 16);
    fido_log_debug("QINV (last 16 bytes):", s);
    fido_log_print_hexdump_debug(p+128-16, 16);
}

void ccid_piv_object_test_read_obj_data(void)
{
    // for research
    bool ret = false;
    size_t s;
    static uint8_t cnt=0;
    switch (cnt) {
        case 0:
            fido_log_debug("PIV object read test start.");
            ret = ccid_piv_object_cert_pauth_get(work_buf, &s);
            cnt++;
            break;
        case 1:
            ret = ccid_piv_object_cert_digsig_get(work_buf, &s);
            cnt++;
            break;
        case 2:
            ret = ccid_piv_object_cert_keyman_get(work_buf, &s);
            cnt++;
            break;
        case 3:
            ret = ccid_piv_object_chuid_get(work_buf, &s);
            cnt++;
            break;
        case 4:
            ret = ccid_piv_object_ccc_get(work_buf, &s);
            cnt++;
            break;
        default:
            fido_log_debug("PIV object read test end.");
            cnt=0;
            return;
    }

    if (ret) {
        fido_log_debug("first 16 bytes:");
        fido_log_print_hexdump_debug(work_buf, 16);
        fido_log_debug("last 16 bytes:");
        fido_log_print_hexdump_debug(work_buf+s-16, 16);
    }
}
#endif // #ifdef CCID_PIV_OBJECT_TEST
