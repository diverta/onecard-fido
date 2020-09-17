/* 
 * File:   ccid_piv_object_import.c
 * Author: makmorit
 *
 * Created on 2020/09/16, 15:28
 */
#include "ccid.h"
#include "ccid_piv.h"
#include "ccid_piv_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

static size_t get_enough_space(uint8_t obj_tag) 
{
    switch (obj_tag) {
        case TAG_OBJ_CHUID:
            // Card Holder Unique Identifier
            return MAX_CHUID_SIZE;
        case TAG_CERT_PAUTH:
            // X.509 Certificate for PIV Authentication
            return MAX_CERT_SIZE;
        case TAG_OBJ_CCC:
            // Card Capability Container
            return MAX_CCC_SIZE;
        case TAG_CERT_DGSIG:
            // X.509 Certificate for Digital Signature
            return MAX_CERT_SIZE;
        case TAG_CERT_KEYMN:
            // X.509 Certificate for Key Management
            return MAX_CERT_SIZE;
        default:
            return 0;
    }
}

uint16_t ccid_piv_object_import(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // 管理コマンドが実行可能でない場合は終了
    if (ccid_piv_admin_mode_get() == false) {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // 受信APDUデータの格納領域
    uint8_t *data = capdu->data;

    // パラメーターのチェック
    if (capdu->p1 != 0x3f || capdu->p2 != 0xff) {
        return SW_WRONG_P1P2;
    }
    if (capdu->lc < 5) {
        return SW_WRONG_LENGTH;
    }
    if (data[0] != 0x5c) {
        return SW_WRONG_DATA;
    }
    if (data[1] != 3 || data[2] != 0x5f || data[3] != 0xc1) {
        // 対象オブジェクトが 0x5fc1xx 形式になっていない場合はエラー
        return SW_FILE_NOT_FOUND;
    }
    uint8_t obj_tag = data[4];
    if (ccid_piv_object_is_obj_tag_exist(obj_tag) == false) {
        return SW_FILE_NOT_FOUND;
    }
    size_t object_size = capdu->lc - 5;
    if (object_size > get_enough_space(obj_tag)) {
        return SW_NOT_ENOUGH_SPACE;
    }

    //
    // TODO: ここに処理を記述
    //

    // 正常終了
    return SW_NO_ERROR;
}
