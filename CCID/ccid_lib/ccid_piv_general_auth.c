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
#include "ccid_piv_pin.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for mbed tls
#include "mbedtls/des.h"

//
// リクエスト／レスポンス格納領域の参照を保持
//
static command_apdu_t  *capdu;
static response_apdu_t *rapdu;
//
// 認証処理に使用する共有情報
//
static uint8_t  auth_ctx[LENGTH_AUTH_STATE];

// 一時作業領域
static uint8_t  work_buf[32];

//
// BER-TLV関連
//  PIVのリクエスト／レスポンスに
//  格納されるデータオブジェクトは、
//  BER-TLV形式で表現されます
//
typedef struct {
    // Witness
    uint16_t wit_pos;
    int16_t  wit_len;
    // Challenge
    uint16_t chl_pos;
    int16_t  chl_len;
    // Response
    uint16_t rsp_pos;
    int16_t  rsp_len;
    // Exponentiation
    uint16_t exp_pos;
    int16_t  exp_len;
} BER_TLV_INFO;

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

void ccid_piv_general_auth_reset_context(void)
{
    // auth context data offset
    //  0: auth state
    //  1: auth key id
    //  2: auth algorism
    //  3: auth challenge (16 bytes)
    auth_ctx[0] = AUTH_STATE_NONE;
    auth_ctx[1] = 0;
    auth_ctx[2] = 0;
    memset(auth_ctx + 3, 0, LENGTH_CHALLENGE);
}

static bool is_key_id_exist(uint8_t id)
{
    switch (id) {
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

static bool tdes_enc(const uint8_t *in, uint8_t *out, const uint8_t *key) 
{
    mbedtls_des_context ctx;
    mbedtls_des_init(&ctx);
    mbedtls_des_setkey_enc(&ctx, key);
    if (mbedtls_des_crypt_ecb(&ctx, in, out) < 0) {
        return false;
    }

    mbedtls_des_free(&ctx);
    return true;
}

static uint16_t mutual_authenticate_request(void)
{
    fido_log_debug("mutual authenticate request");

    // 変数の初期化
    ccid_piv_general_auth_reset_context();
    ccid_piv_admin_mode_set(false);

    // パラメーターのチェック
    if (capdu->p2 != TAG_KEY_CAADM) {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // 管理用キーに対応する暗号アルゴリズムを取得
    uint8_t crypto_alg = ccid_piv_object_card_admin_key_alg_get();

    // 入力データサイズ
    uint8_t challenge_size = TDEA_BLOCK_SIZE;

    // mutual_authenticate_response実行で
    // 使用する情報を退避
    // auth context data offset
    //  0: auth state
    //  1: auth key id
    //  2: auth algorism
    //  3: auth challenge (16 bytes)
    auth_ctx[0] = AUTH_STATE_MUTUAL;
    auth_ctx[1] = capdu->p2;
    auth_ctx[2] = crypto_alg;
    if (challenge_size > sizeof(auth_ctx) - 3) {
        return SW_WRONG_DATA;
    }
    uint8_t *challenge = auth_ctx + 3;
    fido_crypto_generate_random_vector(challenge, challenge_size);

    uint8_t *rdata = rapdu->data;
    rdata[0] = 0x7c;
    rdata[1] = challenge_size + 2;
    rdata[2] = TAG_WITNESS;
    rdata[3] = challenge_size;
    rapdu->len = challenge_size + 4;

    if (crypto_alg == ALG_TDEA_3KEY) {
        // 管理用キーを取得
        size_t size = sizeof(work_buf);
        if (ccid_piv_object_card_admin_key_get(work_buf, &size) == false) {
            return SW_FILE_NOT_FOUND;
        }
        // 管理用キーを使用し、challenge を暗号化
        uint8_t *output_data = rdata + 4;
        if (tdes_enc(challenge, output_data, work_buf) == false) {
            memset(work_buf, 0, sizeof(work_buf));
            return SW_UNABLE_TO_PROCESS;
        }
        memset(work_buf, 0, sizeof(work_buf));

    } else {
        ccid_piv_general_auth_reset_context();
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // 正常終了
    return SW_NO_ERROR;
}

static uint16_t mutual_authenticate_response(BER_TLV_INFO *data_obj_info)
{
    fido_log_debug("mutual authenticate response");

    // 管理用キーに対応する暗号アルゴリズムを取得
    uint8_t crypto_alg = ccid_piv_object_card_admin_key_alg_get();

    // 入力データサイズ
    uint8_t challenge_size = TDEA_BLOCK_SIZE;

    // パラメーターのチェック
    // auth context data offset
    //  0: auth state
    //  1: auth key id
    //  2: auth algorism
    //  3: auth challenge (16 bytes)
    uint8_t *challenge = auth_ctx + 3;
    uint8_t *witness = capdu->data + data_obj_info->wit_pos;
    if (auth_ctx[0] != AUTH_STATE_MUTUAL 
        || auth_ctx[1] != capdu->p2 
        || auth_ctx[2] != crypto_alg 
        || challenge_size != data_obj_info->wit_len 
        || memcmp(challenge, witness, challenge_size) != 0) {
        ccid_piv_general_auth_reset_context();
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }
    if (challenge_size != data_obj_info->chl_len) {
        ccid_piv_general_auth_reset_context();
        return SW_WRONG_LENGTH;
    }

    uint8_t *rdata = rapdu->data;
    rdata[0] = 0x7c;
    rdata[1] = challenge_size + 2;
    rdata[2] = TAG_RESPONSE;
    rdata[3] = challenge_size;
    rapdu->len = challenge_size + 4;

    if (crypto_alg == ALG_TDEA_3KEY) {
        // 管理用キーを取得
        size_t size = sizeof(work_buf);
        if (ccid_piv_object_card_admin_key_get(work_buf, &size) == false) {
            return SW_FILE_NOT_FOUND;
        }
        // 管理用キーを使用し、challenge を暗号化
        uint8_t *input_data = capdu->data + data_obj_info->chl_pos;
        uint8_t *output_data = rdata + 4;
        if (tdes_enc(input_data, output_data, work_buf) == false) {
            memset(work_buf, 0, sizeof(work_buf));
            return SW_UNABLE_TO_PROCESS;
        }
        memset(work_buf, 0, sizeof(work_buf));

    } else {
        ccid_piv_general_auth_reset_context();
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // 変数の初期化
    ccid_piv_general_auth_reset_context();
    ccid_piv_admin_mode_set(true);

    // 正常終了
    return SW_NO_ERROR;
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
    if (is_key_id_exist(capdu->p2) == false) {
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
        func_ret = ccid_piv_authenticate_internal(capdu, rapdu, data_obj_info.chl_pos, data_obj_info.chl_len);

    } else if (data_obj_info.chl_pos > 0 && data_obj_info.chl_len == 0) {
        fido_log_debug("external authenticate request");

    } else if (data_obj_info.rsp_pos > 0 && data_obj_info.rsp_len > 0) {
        fido_log_debug("external authenticate response");

    } else if (data_obj_info.wit_pos > 0 && data_obj_info.wit_len == 0) {
        func_ret = mutual_authenticate_request();

    } else if (data_obj_info.wit_pos > 0 && data_obj_info.wit_len > 0 &&
               data_obj_info.chl_pos > 0 && data_obj_info.chl_len > 0) {
        func_ret = mutual_authenticate_response(&data_obj_info);

    } else if (data_obj_info.rsp_pos > 0 && data_obj_info.rsp_len == 0 &&
               data_obj_info.exp_pos > 0 && data_obj_info.exp_len > 0) {
        func_ret = ccid_piv_authenticate_ecdh_with_kmk(capdu, rapdu, data_obj_info.exp_pos, data_obj_info.exp_len);

    } else {
        // INVALID CASE
        fido_log_error("general authenticate: invalid case");
        ccid_piv_general_auth_reset_context();
        func_ret = SW_WRONG_DATA;
    }
    
    // 正常終了
    return func_ret;
}
