/* 
 * File:   ccid_piv_general_auth.c
 * Author: makmorit
 *
 * Created on 2020/06/03, 14:55
 */
#include "ccid_piv.h"
#include "ccid_piv_general_auth.h"
#include "ccid_piv_object.h"

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
// 業務処理に関する定義
//
// 暗号化アルゴリズム
#define ALG_DEFAULT     0x00
#define ALG_TDEA_3KEY   0x03
#define ALG_RSA_2048    0x07
#define ALG_ECC_256     0x11

// 3-key TDES関連
#define TDEA_BLOCK_SIZE 8

// RSA関連
#define RSA_N_BIT   2048u
#define E_LENGTH    4
#define N_LENGTH    (RSA_N_BIT / 8)
#define PQ_LENGTH   (RSA_N_BIT / 16)

// ECC関連
#define ECC_KEY_SIZE        32
#define ECC_PUB_KEY_SIZE    64

// tags for general auth
#define TAG_WITNESS         0x80
#define TAG_CHALLENGE       0x81
#define TAG_RESPONSE        0x82
#define TAG_EXP             0x85
#define IDX_WITNESS         (TAG_WITNESS - 0x80)
#define IDX_CHALLENGE       (TAG_CHALLENGE - 0x80)
#define IDX_RESPONSE        (TAG_RESPONSE - 0x80)
#define IDX_EXP             (TAG_EXP - 0x80)

#define LENGTH_CHALLENGE    16
#define LENGTH_AUTH_STATE   (5 + LENGTH_CHALLENGE)

// states for auth
#define AUTH_STATE_NONE     0
#define AUTH_STATE_EXTERNAL 1
#define AUTH_STATE_MUTUAL   2

//
// 認証処理に使用する共有情報
//
static uint8_t  auth_ctx[LENGTH_AUTH_STATE];
static uint8_t  crypto_key[24];
static uint8_t  crypto_alg;
static uint16_t input_data_length;
static uint16_t pos_for_tag[6];
static int16_t  len_for_tag[6];

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

static void authenticate_reset(void) 
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

static int get_input_size(uint8_t alg) {
    switch (alg) {
        case ALG_DEFAULT:
        case ALG_TDEA_3KEY:
            return TDEA_BLOCK_SIZE;
        case ALG_RSA_2048:
            return N_LENGTH;
        case ALG_ECC_256:
            return ECC_KEY_SIZE;
        default:
            return 0;
    }
}

static bool is_key_exist(uint8_t id)
{
    switch (id) {
        case 0x9A:
            // PIV_AUTH_KEY_PATH
            return true;
        case 0x9B:
            // CARD_ADMIN_KEY_PATH
            return true;
        case 0x9C:
            // SIG_KEY_PATH
            return true;
        case 0x9D:
            // KEY_MANAGEMENT_KEY_PATH
            return true;
        case 0x9E:
            // CARD_AUTH_KEY_PATH
            return true;
        default:
            return false;
    }
    return true;
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
    authenticate_reset();
    ccid_piv_admin_mode_set(false);

    // パラメーターのチェック
    if (capdu->p2 != 0x9b) {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

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
    if (3 + input_data_length > sizeof(auth_ctx)) {
        return SW_WRONG_DATA;
    }
    uint8_t *challenge = auth_ctx + 3;
    fido_crypto_generate_random_vector(challenge, input_data_length);

    uint8_t *rdata = rapdu->data;
    rdata[0] = 0x7c;
    rdata[1] = input_data_length + 2;
    rdata[2] = TAG_WITNESS;
    rdata[3] = input_data_length;
    rapdu->len = input_data_length + 4;

    if (crypto_alg == ALG_TDEA_3KEY) {
        // 管理用キーを取得
        size_t size = sizeof(crypto_key);
        if (ccid_piv_object_card_admin_key_get(crypto_key, &size) == false) {
            return SW_FILE_NOT_FOUND;
        }
        // 管理用キーを使用し、challenge を暗号化
        if (tdes_enc(challenge, rdata + 4, crypto_key) == false) {
            memset(crypto_key, 0, sizeof(crypto_key));
            return SW_UNABLE_TO_PROCESS;
        }
        memset(crypto_key, 0, sizeof(crypto_key));

    } else {
        authenticate_reset();
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // 正常終了
    return SW_NO_ERROR;
}

static uint16_t mutual_authenticate_response(void)
{
    fido_log_debug("mutual authenticate response");

    // パラメーターのチェック
    // auth context data offset
    //  0: auth state
    //  1: auth key id
    //  2: auth algorism
    //  3: auth challenge (16 bytes)
    uint8_t *challenge = auth_ctx + 3;
    if (auth_ctx[0] != AUTH_STATE_MUTUAL 
        || auth_ctx[1] != capdu->p2 
        || auth_ctx[2] != crypto_alg 
        || input_data_length != len_for_tag[IDX_WITNESS] 
        || memcmp(challenge, capdu->data + pos_for_tag[IDX_WITNESS], input_data_length) != 0) {
        authenticate_reset();
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }
    if (input_data_length != len_for_tag[IDX_CHALLENGE]) {
        authenticate_reset();
        return SW_WRONG_LENGTH;
    }

    uint8_t *rdata = rapdu->data;
    rdata[0] = 0x7c;
    rdata[1] = input_data_length + 2;
    rdata[2] = TAG_RESPONSE;
    rdata[3] = input_data_length;
    rapdu->len = input_data_length + 4;

    if (crypto_alg == ALG_TDEA_3KEY) {
        // 管理用キーを取得
        size_t size = sizeof(crypto_key);
        if (ccid_piv_object_card_admin_key_get(crypto_key, &size) == false) {
            return SW_FILE_NOT_FOUND;
        }
        // 管理用キーを使用し、challenge を復号化
        if (tdes_enc(capdu->data + pos_for_tag[IDX_CHALLENGE], rdata + 4, crypto_key) == false) {
            memset(crypto_key, 0, sizeof(crypto_key));
            return SW_UNABLE_TO_PROCESS;
        }
        memset(crypto_key, 0, sizeof(crypto_key));

    } else {
        authenticate_reset();
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // 変数の初期化
    authenticate_reset();
    ccid_piv_admin_mode_set(true);

    // 正常終了
    return SW_NO_ERROR;
}

static uint16_t ecdh_with_piv_kmk(void)
{
    // ECDH with the PIV KMK
    // Documented in SP800-73-4 Part 2 Appendix A.5
    fido_log_debug("ECDH with the PIV KMK");

    // 正常終了
    return SW_NO_ERROR;
}

static bool parse_pos_and_length_for_tags(uint8_t *data)
{
    memset(pos_for_tag, 0, sizeof(pos_for_tag));
    memset(len_for_tag, 0, sizeof(len_for_tag));
    uint16_t dat_len = tlv_get_length(data + 1);
    uint16_t dat_pos = 1 + tlv_length_size(dat_len);
    while (dat_pos < capdu->lc) {
        uint8_t tag = data[dat_pos++];
        if (tag != 0x80 && tag != 0x81 && tag != 0x82 && tag != 0x85) {
            return false;
        }
        len_for_tag[tag - 0x80] = tlv_get_length(data + dat_pos);
        dat_pos += tlv_length_size(len_for_tag[tag - 0x80]);
        pos_for_tag[tag - 0x80] = dat_pos;
        dat_pos += len_for_tag[tag - 0x80];
        fido_log_debug("Tag 0x%02X, pos[%d]: %d, len[%d]: %d", 
            tag, tag - 0x80, pos_for_tag[tag - 0x80], tag - 0x80, len_for_tag[tag - 0x80]);
    }
    return true;
}

uint16_t piv_ins_general_authenticate(command_apdu_t *c_apdu, response_apdu_t *r_apdu)
{
    // リクエスト／レスポンス格納領域の参照を保持
    capdu = c_apdu;
    rapdu = r_apdu;

    // パラメーターのチェック
    if (capdu->data[0] != 0x7C) {
        return SW_WRONG_DATA;
    }
    if (is_key_exist(capdu->p2) == false) {
        return SW_WRONG_P1P2;
    }

    // 管理用キーに対応する暗号アルゴリズムを取得し、
    // 入力データサイズを判定
    crypto_alg = ccid_piv_object_card_admin_key_alg_get();
    input_data_length = get_input_size(crypto_alg);

    // データ格納領域の参照
    uint8_t *data = capdu->data;

    // データ要素ごとの長さ／格納位置を解析
    if (parse_pos_and_length_for_tags(data) == false) {
        return SW_WRONG_DATA;
    }

    // アプリケーション認証処理を実行
    uint16_t func_ret = SW_NO_ERROR;
    if (pos_for_tag[IDX_WITNESS] == 0 && pos_for_tag[IDX_CHALLENGE] > 0 
        && len_for_tag[IDX_CHALLENGE] > 0 && pos_for_tag[IDX_RESPONSE] > 0 && len_for_tag[IDX_RESPONSE] == 0) {
        fido_log_debug("internal authenticate");

    } else if (pos_for_tag[IDX_CHALLENGE] > 0 && len_for_tag[IDX_CHALLENGE] == 0) {
        fido_log_debug("external authenticate request");

    } else if (pos_for_tag[IDX_RESPONSE] > 0 && len_for_tag[IDX_RESPONSE] > 0) {
        fido_log_debug("external authenticate response");

    } else if (pos_for_tag[IDX_WITNESS] > 0 && len_for_tag[IDX_WITNESS] == 0) {
        func_ret = mutual_authenticate_request();

    } else if (pos_for_tag[IDX_WITNESS] > 0 && len_for_tag[IDX_WITNESS] > 0 
        && pos_for_tag[IDX_CHALLENGE] > 0 && len_for_tag[IDX_CHALLENGE] > 0) {
        func_ret = mutual_authenticate_response();

    } else if (pos_for_tag[IDX_RESPONSE] > 0 && len_for_tag[IDX_RESPONSE] == 0 
        && pos_for_tag[IDX_EXP] > 0 && len_for_tag[IDX_EXP] > 0) {
        func_ret = ecdh_with_piv_kmk();

    } else {
        // INVALID CASE
        fido_log_error("general authenticate: invalid case");
        authenticate_reset();
        func_ret = SW_WRONG_DATA;
    }
    
    // 正常終了
    return func_ret;
}
