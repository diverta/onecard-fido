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

//
// テスト用の仮データ
//  正式には、yubico-piv-tool 等を利用し、
//  Flash ROMにインストールした証明書等データを
//  使用することになります。
//
#define CCID_PIV_OBJECT_TEST false
#if CCID_PIV_OBJECT_TEST
static char *CHUID_TEMP = "533B3019D4E739DA739CED39CE739D836858210842108421C84210C3EB34104C8D536A86AA98A5CE20D53557776E58350832303330303130313E00FE00";
static char *CCC_TEMP   = "5333f015a000000116ff02d4bfab488d66fa69ae507ee5f8daf10121f20121f300f40100f50110f600f700fa00fb00fc00fd00fe00";
static char *TAG05_TEMP = "538201957082018C308201883082012EA003020102020900AA8C96AB7B237546300A06082A8648CE3D040302301F311D301B06035504030C14706976617574682E6469766572742E636F2E6A70301E170D3230303732373033333733325A170D3231303732373033333733325A301F311D301B06035504030C14706976617574682E6469766572742E636F2E6A703059301306072A8648CE3D020106082A8648CE3D03010703420004642C07D809244DBD83039C799D095CFF9E831C2AA28A2F9E97C72408742455C42A525355695761AB4B4C8440B73913CD26C6B94BB2D3F8CA3C71C8E061F07054A3533051301D0603551D0E041604144AD8A8F4A36127736A7452AB0C8B424D0493B820301F0603551D230418301680144AD8A8F4A36127736A7452AB0C8B424D0493B820300F0603551D130101FF040530030101FF300A06082A8648CE3D0403020348003045022100AC6CDCBA0C17A518460E08236B1FAC02A9BB06F998D511826F6A5F2DC7F1EFFB02207EEBDBF47B43484DDFEC9D9AC0161412CFCBC9D4A8A86802F6D4BE9FDDA62869710100FE00";
static char *TAG0A_TEMP = "538201947082018B308201873082012EA003020102020900C2DD04F6172EACF2300A06082A8648CE3D040302301F311D301B06035504030C146469677369676E2E6469766572742E636F2E6A70301E170D3230303732373033333734355A170D3231303732373033333734355A301F311D301B06035504030C146469677369676E2E6469766572742E636F2E6A703059301306072A8648CE3D020106082A8648CE3D0301070342000483C0E6B87EE7C139C0864DB29647A35862367489C5AF540C30B63A6269B56A1830890DFB2538AAC9D9981C17C5AF11E24116723454706316E19C688BFA509827A3533051301D0603551D0E041604149E5C213F4A9671A3BDFA3295D9B109F5656B243E301F0603551D230418301680149E5C213F4A9671A3BDFA3295D9B109F5656B243E300F0603551D130101FF040530030101FF300A06082A8648CE3D04030203470030440220049470B3A571276E6EEF3444261F8AA95CAF06E26FA3CE7B9562767BA3EC0F2A022004DA8F86138F5E08124D673914DBBBB05299FF7DFD3E0D8C2AB4B1B1594E09BC710100FE00";
static char *TAG0B_TEMP = "538201957082018C308201883082012EA003020102020900E7ADC7F137C0E21B300A06082A8648CE3D040302301F311D301B06035504030C146B65796D676D742E6469766572742E636F2E6A70301E170D3230303732373033333735375A170D3231303732373033333735375A301F311D301B06035504030C146B65796D676D742E6469766572742E636F2E6A703059301306072A8648CE3D020106082A8648CE3D030107034200042CE9A2A7405CC00105AB0BA738C75C2FCF5E6C90FBBC534EF4F8BB0433CA9CB0BFEBA6713FE4340C9AD441F4E610C62CE7A4B800D2048475769D170B8D9689C8A3533051301D0603551D0E041604144DAFB7458381AEFEDC813136274CBE4749A6EADE301F0603551D230418301680144DAFB7458381AEFEDC813136274CBE4749A6EADE300F0603551D130101FF040530030101FF300A06082A8648CE3D0403020348003045022100F59E93D2D4CDB8D98594834D115A4D9A4D87FB2B893C184180D0617786DB500202203CBAB7C403E96F01FBB07729A9F8E36923227EB060146C0D13F6D49BFB473C6E710100FE00";
#endif

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

static bool read_private_key(uint8_t tag, uint8_t alg, uint8_t *buffer, size_t *size)
{
    // 秘密鍵データをFlash ROMから読出し
    bool is_exist;
    if (ccid_flash_piv_object_private_key_read(tag, alg, buffer, size, &is_exist) == false) {
        // 読出しが失敗した場合はエラー
        fido_log_error("Private Key for PIV application read fail: tag=0x02x", tag);
        return false;
    }

    if (is_exist == false) {
        // Flash ROMに登録されていない場合はエラー
        fido_log_error("Private Key for PIV application is not registered: tag=0x02x", tag);
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
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(TAG05_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("X.509 Certificate for PIV Authentication is requested (%d bytes)", *size);
    return false;
#endif
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
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(TAG0A_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("X.509 Certificate for Digital Signature is requested (%d bytes)", *size);
    return false;
#endif
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
#if CCID_PIV_OBJECT_TEST
    *size = convert_hexstring_to_bytes(TAG0B_TEMP, buffer);
    return true;
#else
    // 後日正式に実装予定です。
    fido_log_debug("X.509 Certificate for Key Management is requested (%d bytes)", *size);
    return false;
#endif
}

bool ccid_piv_object_card_admin_key_get(uint8_t *buffer, size_t *size, uint8_t *alg)
{
    // パスワードをFlash ROMから読出し
    bool is_exist;
    if (ccid_flash_piv_object_card_admin_key_read(buffer, size, alg, &is_exist) == false) {
        // 読出しが失敗した場合はエラー
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
        case TAG_KEY_CAUTH:
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
