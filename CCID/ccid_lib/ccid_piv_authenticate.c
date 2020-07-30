/* 
 * File:   ccid_piv_authenticate.c
 * Author: makmorit
 *
 * Created on 2020/07/23, 16:32
 */
#include "ccid_piv.h"
#include "ccid_piv_general_auth.h"
#include "ccid_piv_authenticate.h"
#include "ccid_piv_object.h"
#include "ccid_piv_pin.h"

// for ECDSA
#include "u2f_signature.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// リクエスト／レスポンス格納領域の参照を保持
//
static command_apdu_t  *capdu;
static response_apdu_t *rapdu;

// 一時作業領域
static uint8_t work_buf[1024];
static uint8_t digest[SHA_256_HASH_SIZE];

static uint8_t *pubkey_in_certificate(uint8_t *cert_data, size_t cert_data_length)
{
    for (size_t i = 3; i < cert_data_length; i++) {
        if (cert_data[i-3] == 0x03 && cert_data[i-2] == 0x42 &&
            cert_data[i-1] == 0x00 && cert_data[i]   == 0x04) {
            // 03 42 00 04 というシーケンスが発見されたら、
            // その先頭アドレスを戻す
            return (cert_data + i + 1);
        }
    }
    fido_log_error("pubkey_in_certificate failed: Public key not found");
    return NULL;
}

static uint16_t generate_ecdsa_sign(uint8_t *input_data, size_t input_size, uint8_t *output_data, size_t *output_size)
{
    // パラメーターのチェック
    uint8_t digest_size = SHA_256_HASH_SIZE;
    if (input_size > digest_size) {
        return SW_WRONG_DATA;
    }

    // 該当のスロットから、EC鍵を読込
    size_t s;
    if (ccid_piv_object_key_pauth_get(work_buf, &s) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 入力チャレンジがハッシュ長より短い場合は、先頭から 0 を埋める
    uint8_t offset = digest_size - input_size;
    memset(digest, 0, sizeof(digest));
    memcpy(digest + offset, input_data, input_size);

    // ECDSA署名を生成
    uint8_t *privkey_be = work_buf;
    *output_size = ECDSA_SIGNATURE_SIZE;
    fido_crypto_ecdsa_sign(privkey_be, digest, digest_size, output_data, output_size);
    memset(work_buf, 0, sizeof(work_buf));

    // 該当のスロットから、証明書を読込
    if (ccid_piv_object_cert_pauth_get(work_buf, &s) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 証明書から公開鍵を抽出
    uint8_t *pubkey_be = pubkey_in_certificate(work_buf, s);
    if (pubkey_be == NULL) {
        return SW_UNABLE_TO_PROCESS;
    }

    // ECDSA署名を検証
    s = *output_size;
    if (fido_crypto_ecdsa_sign_verify(pubkey_be, digest, digest_size, output_data, s) == false) {
        // 署名検証失敗の場合終了
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

uint16_t ccid_piv_authenticate_internal(command_apdu_t *c_apdu, response_apdu_t *r_apdu, uint8_t challenge_pos, uint8_t challenge_size)
{
    // リクエスト／レスポンス格納領域の参照を保持
    capdu = c_apdu;
    rapdu = r_apdu;

    fido_log_debug("internal authenticate");

    // 変数の初期化
    ccid_piv_general_auth_reset_context();
    uint8_t key_alg = capdu->p1;
    uint8_t key_id  = capdu->p2;

    // パラメーターのチェック
    if (key_alg != ALG_ECC_256) {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }
    if (key_id != TAG_KEY_CAUTH && ccid_piv_pin_is_validated() == false) {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }
    if (key_id == TAG_KEY_KEYMN) {
        ccid_piv_pin_set_validated(false);
    }

    uint8_t *cdata = capdu->data;
    uint8_t *rdata = rapdu->data;
    uint8_t *input_data = cdata + challenge_pos;
    uint8_t *output_data = rdata + 4;
    size_t input_size = challenge_size;
    size_t output_size = 0;

    // 該当のスロットから、EC秘密鍵を読込み、応答を生成
    uint16_t ret = generate_ecdsa_sign(input_data, input_size, output_data, &output_size);
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
    return SW_NO_ERROR;
}

uint16_t ccid_piv_authenticate_ecdh_with_kmk(command_apdu_t *c_apdu, response_apdu_t *r_apdu, uint8_t pubkey_pos, uint8_t pubkey_size)
{
    // リクエスト／レスポンス格納領域の参照を保持
    capdu = c_apdu;
    rapdu = r_apdu;

    // ECDH with the PIV KMK
    // Documented in SP800-73-4 Part 2 Appendix A.5
    fido_log_debug("ECDH with the PIV KMK");

    // 変数の初期化
    ccid_piv_general_auth_reset_context();

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
    if (ccid_piv_object_key_keyman_get(work_buf, &size) == false) {
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
