/* 
 * File:   ccid_piv_internal_auth.c
 * Author: makmorit
 *
 * Created on 2020/07/23, 16:32
 */
#include "ccid_piv.h"
#include "ccid_piv_general_auth.h"
#include "ccid_piv_internal_auth.h"
#include "ccid_piv_object.h"
#include "ccid_piv_pin.h"

// for ECDSA
#include "u2f_signature.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// 業務処理に関する定義
//
// 暗号化アルゴリズム
#define ALG_ECC_256     0x11

//
// リクエスト／レスポンス格納領域の参照を保持
//
static command_apdu_t  *capdu;
static response_apdu_t *rapdu;

// 一時作業領域
static uint8_t privkey_be[RAW_PRIVATE_KEY_SIZE];
static uint8_t digest[SHA_256_HASH_SIZE];

uint16_t generate_ecdsa_sign(uint8_t *input_data, size_t input_size, uint8_t *output_data, size_t *output_size)
{
    // キー長を取得
    uint8_t digest_size = SHA_256_HASH_SIZE;
    if (input_size > digest_size) {
        return SW_WRONG_DATA;
    }

    // 該当のスロットから、EC鍵を読込
    size_t s;
    if (ccid_piv_object_key_pauth_get(privkey_be, &s) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 入力チャレンジがキー長より短い場合は、先頭から 0 を埋める
    uint8_t offset = digest_size - input_size;
    memset(digest, 0, sizeof(digest));
    memcpy(digest + offset, input_data, input_size);

    // ECDSA署名を生成
    *output_size = ECDSA_SIGNATURE_SIZE;
    fido_crypto_ecdsa_sign(privkey_be, digest, digest_size, output_data, output_size);
    memset(privkey_be, 0, sizeof(privkey_be));

    // ASN.1形式署名を格納する領域を準備
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

uint16_t ccid_piv_internal_auth(command_apdu_t *c_apdu, response_apdu_t *r_apdu, uint8_t challenge_pos, uint8_t challenge_size)
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
    if (key_id != 0x9E && ccid_piv_pin_is_validated() == false) {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }
    if (key_id == 0x9D) {
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
    rdata[2] = 0x82;
    rdata[3] = output_size;
    rapdu->len = output_size + 4;

    // 正常終了
    return SW_NO_ERROR;
}
