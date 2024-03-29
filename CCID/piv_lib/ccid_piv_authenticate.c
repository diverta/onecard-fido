/* 
 * File:   ccid_piv_authenticate.c
 * Author: makmorit
 *
 * Created on 2020/07/23, 16:32
 */
#include "ccid.h"
#include "ccid_define.h"
#include "ccid_piv.h"
#include "ccid_piv_define.h"
#include "ccid_piv_general_auth.h"
#include "ccid_piv_object.h"
#include "ccid_piv_pin.h"

// for ECDSA
#include "u2f_signature.h"

// for fido_extract_pubkey_in_certificate
#include "fido_common.h"

// for fido_command_generate_random_vector
#include "fido_command_common.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_piv_authenticate);
#endif

//
// リクエスト／レスポンス格納領域の参照を保持
//
static command_apdu_t  *capdu;
static response_apdu_t *rapdu;

// 一時作業領域
static uint8_t work_buf[1024];
static uint8_t digest[SHA_256_HASH_SIZE];

//
// 認証処理に使用する共有情報
//
static uint8_t  auth_ctx[LENGTH_AUTH_STATE];

void ccid_piv_authenticate_reset_context(void)
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
    memset(work_buf, 0, sizeof(work_buf));
}

static uint16_t generate_ecdsa_sign(uint8_t tag, uint8_t *input_data, size_t input_size, uint8_t *output_data, size_t *output_size)
{
    // パラメーターのチェック
    uint8_t digest_size = SHA_256_HASH_SIZE;
    if (input_size > digest_size) {
        return SW_WRONG_DATA;
    }

    // 該当のスロットから、EC鍵を読込
    size_t s;
    if (ccid_piv_object_read_private_key(tag, ALG_ECC_256, work_buf, &s) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 入力チャレンジがハッシュ長より短い場合は、先頭から 0 を埋める
    uint8_t offset = digest_size - input_size;
    memset(digest, 0, sizeof(digest));
    memcpy(digest + offset, input_data, input_size);

    // ECDSA署名を生成
    uint8_t *privkey_be = work_buf;
    *output_size = ECDSA_SIGNATURE_SIZE;
    bool b = fido_crypto_ecdsa_sign(privkey_be, digest, digest_size, output_data, output_size);
    memset(work_buf, 0, sizeof(work_buf));
    if (b == false) {
        // 署名生成失敗の場合終了
        return SW_UNABLE_TO_PROCESS;
    }

    if (u2f_signature_convert_to_asn1(output_data) == false) {
        // 生成された署名をASN.1形式署名に変換する
        // 変換失敗の場合終了
        return SW_UNABLE_TO_PROCESS;
    }

    // データとサイズを取得
    *output_size = u2f_signature_data_size();
    memcpy(output_data, u2f_signature_data_buffer(), *output_size);

    // 正常終了
    return SW_NO_ERROR;
}

static uint16_t authenticate_internal_ECC_256(uint8_t tag, uint8_t *input_data, size_t input_size)
{
    uint8_t *rdata = rapdu->data;
    uint8_t *output_data = rdata + 4;
    size_t output_size = 0;

    // 該当のスロットから、EC秘密鍵を読込み、応答を生成
    uint16_t ret = generate_ecdsa_sign(tag, input_data, input_size, output_data, &output_size);
    if (ret != SW_NO_ERROR) {
        return ret;
    }

    // レスポンスデータを生成
    rdata[0] = 0x7c;
    rdata[1] = output_size + 2;
    rdata[2] = TAG_RESPONSE;
    rdata[3] = output_size;
    rapdu->len = output_size + 4;

    // 正常終了
    fido_log_debug("internal authenticate (ECCP256) done");
    return SW_NO_ERROR;
}

static uint16_t authenticate_internal_RSA2048(uint8_t tag, uint8_t *input_data, size_t input_size)
{
    // パラメーターのチェック
    if (input_size != RSA2048_N_LENGTH) {
        return SW_WRONG_DATA;
    }

    // 該当のスロットから鍵を読込
    size_t s;
    if (ccid_piv_object_read_private_key(tag, ALG_RSA_2048, work_buf, &s) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // レスポンス格納領域の参照を取得
    uint8_t *rdata = rapdu->data;
    uint8_t *output_data = rdata + 8;

    // キープアライブタイマーを開始
    fido_repeat_process_timer_start(1000, ccid_response_time_extension);

    // 署名を生成
    bool ret = ccid_crypto_rsa_private(work_buf, input_data, output_data);
    memset(work_buf, 0, sizeof(work_buf));

    // キープアライブタイマーを停止
    fido_repeat_process_timer_stop();
    if (ret == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // レスポンスデータを生成
    rdata[0] = 0x7c;
    rdata[1] = 0x82;
    rdata[2] = HI(RSA2048_N_LENGTH + 4);
    rdata[3] = LO(RSA2048_N_LENGTH + 4);
    rdata[4] = TAG_RESPONSE;
    rdata[5] = 0x82;
    rdata[6] = HI(RSA2048_N_LENGTH);
    rdata[7] = LO(RSA2048_N_LENGTH);
    rapdu->len = RSA2048_N_LENGTH + 8;

    // 正常終了
    fido_log_debug("internal authenticate (RSA2048) done");
    return SW_NO_ERROR;
}

uint16_t ccid_piv_authenticate_internal(void *p_capdu, void *p_rapdu, void *data_obj_ber_tlv_info)
{
    // リクエスト／レスポンス格納領域の参照を保持
    capdu = p_capdu;
    rapdu = p_rapdu;

    fido_log_debug("internal authenticate");

    // 変数の初期化
    ccid_piv_authenticate_reset_context();
    BER_TLV_INFO *data_obj_info = (BER_TLV_INFO *)data_obj_ber_tlv_info;
    uint8_t challenge_pos = data_obj_info->chl_pos;
    size_t  challenge_size = data_obj_info->chl_len;
    uint8_t key_alg = capdu->p1;
    uint8_t key_tag  = capdu->p2;

    // TODO:
    //   Card authenticateをサポートする場合、
    //   このコードを有効化します
    // if (key_id != TAG_KEY_CAUTH && ccid_piv_pin_is_validated() == false) {
    //     return SW_SECURITY_STATUS_NOT_SATISFIED;
    // }

    if (key_tag == TAG_KEY_KEYMN) {
        ccid_piv_pin_set_validated(false);
    }

    // 入力データの参照とサイズを取得
    uint8_t *cdata = capdu->data;
    uint8_t *input_data = cdata + challenge_pos;
    size_t input_size = challenge_size;

    // アルゴリズムに対応する処理を実行
    if (key_alg == ALG_ECC_256) {
        return authenticate_internal_ECC_256(key_tag, input_data, input_size);
    } else if (key_alg == ALG_RSA_2048) {
        return authenticate_internal_RSA2048(key_tag, input_data, input_size);
    } else {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }
}

uint16_t ccid_piv_authenticate_ecdh_with_kmk(void *p_capdu, void *p_rapdu, void *data_obj_ber_tlv_info)
{
    // リクエスト／レスポンス格納領域の参照を保持
    capdu = p_capdu;
    rapdu = p_rapdu;

    // ECDH with the PIV KMK
    // Documented in SP800-73-4 Part 2 Appendix A.5
    fido_log_debug("ECDH with the PIV KMK");

    // 変数の初期化
    ccid_piv_authenticate_reset_context();
    BER_TLV_INFO *data_obj_info = (BER_TLV_INFO *)data_obj_ber_tlv_info;
    uint8_t pubkey_pos = data_obj_info->exp_pos;
    uint8_t pubkey_size = data_obj_info->exp_len;

    // パラメーターのチェック
    if (capdu->p2 != TAG_KEY_KEYMN || ccid_piv_pin_is_validated() == false) {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }
    if (pubkey_size != RAW_PUBLIC_KEY_SIZE + 1) {
        return SW_WRONG_DATA;
    }
    if (capdu->p2 == TAG_KEY_KEYMN) {
        ccid_piv_pin_set_validated(false);
    }

    // 該当のスロットから、EC鍵を読込
    size_t size;
    if (ccid_piv_object_read_private_key(capdu->p2, ALG_ECC_256, work_buf, &size) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 受信した公開鍵
    uint8_t *cdata = capdu->data;
    uint8_t *rdata = rapdu->data;
    uint8_t *pubkey = cdata + pubkey_pos + 1;
    uint8_t *ecdh = rdata + 4;

    // 秘密鍵と公開鍵から共通鍵を生成
    if (fido_crypto_calculate_ecdh(work_buf, pubkey, ecdh, &size) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // レスポンスデータを生成
    rdata[0] = 0x7c;
    rdata[1] = size + 2;
    rdata[2] = TAG_RESPONSE;
    rdata[3] = size;
    rapdu->len = size + 4;

    // 正常終了
    return SW_NO_ERROR;
}

static void ccid_piv_authenticate_mutual_request_resume(void);

uint16_t ccid_piv_authenticate_mutual_request(void *p_capdu, void *p_rapdu, void *data_obj_ber_tlv_info)
{
    // リクエスト／レスポンス格納領域の参照を保持
    capdu = p_capdu;
    rapdu = p_rapdu;

    fido_log_debug("mutual authenticate request");

    // 変数の初期化
    ccid_piv_authenticate_reset_context();
    ccid_piv_admin_mode_set(false);
    (void)data_obj_ber_tlv_info;

    // パラメーターのチェック
    if (capdu->p2 != TAG_KEY_CAADM) {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // `resume_mutual_request`内で実行される
    // `fido_command_generate_random_vector`の実行事前に、
    // ランダムベクターの生成を指示
    // （完了するまで、レスポンスを抑止）
    fido_crypto_random_pre_generate(ccid_piv_authenticate_mutual_request_resume);
    ccid_piv_apdu_resume_prepare(p_capdu, p_rapdu);
    return SW_NO_ERROR;
}

static uint16_t resume_mutual_request(void)
{
    // 管理用キーを取得
    fido_log_debug("mutual authenticate request resume");
    size_t size = sizeof(work_buf);
    uint8_t crypto_alg;
    if (ccid_piv_object_card_admin_key_get(work_buf, &size, &crypto_alg) == false) {
        return SW_FILE_NOT_FOUND;
    }

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
        ccid_piv_authenticate_reset_context();
        return SW_WRONG_DATA;
    }
    uint8_t *challenge = auth_ctx + 3;
    fido_command_generate_random_vector(challenge, challenge_size);

    // レスポンスデータを生成
    uint8_t *rdata = rapdu->data;
    rdata[0] = 0x7c;
    rdata[1] = challenge_size + 2;
    rdata[2] = TAG_WITNESS;
    rdata[3] = challenge_size;
    rapdu->len = challenge_size + 4;

    if (crypto_alg == ALG_TDEA_3KEY) {
        // 管理用キーを使用し、challenge を暗号化
        uint8_t *output_data = rdata + 4;
        if (fido_crypto_tdes_enc(challenge, output_data, work_buf) == false) {
            ccid_piv_authenticate_reset_context();
            return SW_UNABLE_TO_PROCESS;
        }
        memset(work_buf, 0, sizeof(work_buf));

    } else {
        ccid_piv_authenticate_reset_context();
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // 正常終了
    return SW_NO_ERROR;
}

static void ccid_piv_authenticate_mutual_request_resume(void)
{
    // `fido_command_generate_random_vector`を含む
    // 後続処理を実行
    uint16_t sw = resume_mutual_request();

    // レスポンス処理を指示
    ccid_piv_apdu_resume_process(sw);        
}

uint16_t ccid_piv_authenticate_mutual_response(void *p_capdu, void *p_rapdu, void *data_obj_ber_tlv_info)
{
    // リクエスト／レスポンス格納領域の参照を保持
    capdu = p_capdu;
    rapdu = p_rapdu;

    fido_log_debug("mutual authenticate response");

    // 管理用キーを取得
    size_t size = sizeof(work_buf);
    uint8_t crypto_alg;
    if (ccid_piv_object_card_admin_key_get(work_buf, &size, &crypto_alg) == false) {
        return SW_FILE_NOT_FOUND;
    }

    // 入力データサイズ
    uint8_t challenge_size = TDEA_BLOCK_SIZE;

    // パラメーターのチェック
    // auth context data offset
    //  0: auth state
    //  1: auth key id
    //  2: auth algorism
    //  3: auth challenge (16 bytes)
    BER_TLV_INFO *data_obj_info = (BER_TLV_INFO *)data_obj_ber_tlv_info;
    uint8_t *challenge = auth_ctx + 3;
    uint8_t *witness = capdu->data + data_obj_info->wit_pos;
    if (auth_ctx[0] != AUTH_STATE_MUTUAL 
        || auth_ctx[1] != capdu->p2 
        || auth_ctx[2] != crypto_alg 
        || challenge_size != data_obj_info->wit_len 
        || memcmp(challenge, witness, challenge_size) != 0) {
        ccid_piv_authenticate_reset_context();
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }
    if (challenge_size != data_obj_info->chl_len) {
        ccid_piv_authenticate_reset_context();
        return SW_WRONG_LENGTH;
    }

    // レスポンスデータを生成
    uint8_t *rdata = rapdu->data;
    rdata[0] = 0x7c;
    rdata[1] = challenge_size + 2;
    rdata[2] = TAG_RESPONSE;
    rdata[3] = challenge_size;
    rapdu->len = challenge_size + 4;

    if (crypto_alg == ALG_TDEA_3KEY) {
        // 管理用キーを使用し、challenge を暗号化
        uint8_t *input_data = capdu->data + data_obj_info->chl_pos;
        uint8_t *output_data = rdata + 4;
        if (fido_crypto_tdes_enc(input_data, output_data, work_buf) == false) {
            ccid_piv_authenticate_reset_context();
            return SW_UNABLE_TO_PROCESS;
        }

    } else {
        ccid_piv_authenticate_reset_context();
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // 変数の初期化
    ccid_piv_authenticate_reset_context();
    ccid_piv_admin_mode_set(true);

    // 正常終了
    return SW_NO_ERROR;
}
