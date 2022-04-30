/* 
 * File:   ccid_piv_general_auth.c
 * Author: makmorit
 *
 * Created on 2020/06/03, 14:55
 */
#include "ccid_piv.h"
#include "ccid_piv_general_auth.h"
#include "ccid_piv_authenticate.h"
#include "ccid_piv_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_piv_general_auth);
#endif

//
// リクエスト／レスポンス格納領域の参照を保持
//
static command_apdu_t  *capdu;
static response_apdu_t *rapdu;

//
// BER-TLV関連
//  PIVのリクエスト／レスポンスに
//  格納されるデータオブジェクトは、
//  BER-TLV形式で表現されます
//
static uint16_t tlv_get_length(const uint8_t *data) 
{
    if (data[0] < 0x80) {
        return data[0];
    }
    if (data[0] == 0x81) {
        return data[1];
    }
    if (data[0] == 0x82) {
        return (uint16_t)(data[1] << 8u) | data[2];
    }
    return 0;
}

static uint8_t tlv_length_size(uint16_t length) 
{
  if (length < 128) {
      return 1;
  } else if (length < 256) {
      return 2;
  } else {
      return 3;
  }
}

static bool parse_ber_tlv_info(uint8_t *data, BER_TLV_INFO *info)
{
    memset(info, 0, sizeof(BER_TLV_INFO));

    uint16_t dat_len = tlv_get_length(data + 1);
    uint16_t dat_pos = 1 + tlv_length_size(dat_len);

    while (dat_pos < capdu->lc) {
        uint8_t tag = data[dat_pos++];
        int16_t len = tlv_get_length(data + dat_pos);
        dat_pos += tlv_length_size(len);
        uint16_t pos = dat_pos;
        dat_pos += len;
        fido_log_debug("Tag 0x%02X, pos: %d, len: %d", tag, pos, len);

        switch (tag) {
            case TAG_WITNESS:
                info->wit_len = len;
                info->wit_pos = pos;
                break;
            case TAG_CHALLENGE:
                info->chl_len = len;
                info->chl_pos = pos;
                break;
            case TAG_RESPONSE:
                info->rsp_len = len;
                info->rsp_pos = pos;
                break;
            case TAG_EXPONENT:
                info->exp_len = len;
                info->exp_pos = pos;
                break;
            default:
                return false;
        }
    }

    return true;
}

uint16_t ccid_piv_general_authenticate(command_apdu_t *c_apdu, response_apdu_t *r_apdu)
{
    // リクエスト／レスポンス格納領域の参照を保持
    capdu = c_apdu;
    rapdu = r_apdu;

    // パラメーターのチェック
    if (capdu->data[0] != 0x7C) {
        return SW_WRONG_DATA;
    }
    if (ccid_piv_object_is_key_tag_exist(capdu->p2) == false) {
        return SW_WRONG_P1P2;
    }

    // 各データオブジェクトの長さ／格納位置を解析
    BER_TLV_INFO data_obj_info;
    if (parse_ber_tlv_info(capdu->data, &data_obj_info) == false) {
        return SW_WRONG_DATA;
    }

    // アプリケーション認証処理を実行
    uint16_t func_ret = SW_NO_ERROR;
    if (data_obj_info.wit_pos == 0 &&
        data_obj_info.chl_pos > 0 && data_obj_info.chl_len > 0 &&
        data_obj_info.rsp_pos > 0 && data_obj_info.rsp_len == 0) {
        func_ret = ccid_piv_authenticate_internal(capdu, rapdu, &data_obj_info);

    } else if (data_obj_info.chl_pos > 0 && data_obj_info.chl_len == 0) {
        fido_log_debug("external authenticate request");

    } else if (data_obj_info.rsp_pos > 0 && data_obj_info.rsp_len > 0) {
        fido_log_debug("external authenticate response");

    } else if (data_obj_info.wit_pos > 0 && data_obj_info.wit_len == 0) {
        func_ret = ccid_piv_authenticate_mutual_request(capdu, rapdu, &data_obj_info);

    } else if (data_obj_info.wit_pos > 0 && data_obj_info.wit_len > 0 &&
               data_obj_info.chl_pos > 0 && data_obj_info.chl_len > 0) {
        func_ret = ccid_piv_authenticate_mutual_response(capdu, rapdu, &data_obj_info);

    } else if (data_obj_info.rsp_pos > 0 && data_obj_info.rsp_len == 0 &&
               data_obj_info.exp_pos > 0 && data_obj_info.exp_len > 0) {
        func_ret = ccid_piv_authenticate_ecdh_with_kmk(capdu, rapdu, &data_obj_info);

    } else {
        // INVALID CASE
        fido_log_error("general authenticate: invalid case");
        ccid_piv_authenticate_reset_context();
        func_ret = SW_WRONG_DATA;
    }
    
    // 正常終了
    return func_ret;
}
