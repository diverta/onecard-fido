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

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// 業務処理に関する定義
//
// 暗号化アルゴリズム
#define ALG_RSA_2048    0x07
#define ALG_ECC_256     0x11
#define ALG_ECC_384     0x14

// RSA関連
#define RSA_N_BIT   2048u
#define E_LENGTH    4
#define N_LENGTH    (RSA_N_BIT / 8)
#define PQ_LENGTH   (RSA_N_BIT / 16)

// ECC関連
#define ECC_256_PRI_KEY_SIZE 32
#define ECC_256_PUB_KEY_SIZE 64
#define ECC_384_PRI_KEY_SIZE 48
#define ECC_384_PUB_KEY_SIZE 96

#define TAG_RESPONSE         0x82

//
// リクエスト／レスポンス格納領域の参照を保持
//
static command_apdu_t  *capdu;
static response_apdu_t *rapdu;

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
    if (key_id != 0x9E && ccid_piv_pin_is_validated() == false) {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }
    if (key_id == 0x9D) {
        ccid_piv_pin_set_validated(false);
    }

    uint8_t *cdata = capdu->data;
    uint8_t *rdata = rapdu->data;
    uint8_t *input_data = cdata + challenge_pos;
    uint8_t input_size = challenge_size;

    if (key_alg == ALG_RSA_2048) {
        size_t response_size = N_LENGTH;
        if (response_size != input_size) {
            return SW_WRONG_DATA;
        }

        // 変数初期化
        uint8_t *output_data = rdata + 8;
        memset(output_data, 0, response_size);
        
        // 該当のスロットから、RSA秘密鍵を読込み、応答チャレンジを生成
        //rsa_key_t key;
        //if (read_file(key_path, &key, 0, sizeof(rsa_key_t)) < 0) {
        //    return SW_UNABLE_TO_PROCESS;
        //}

        //if (rsa_private(&key, input_data, output_data) < 0) {
        //    memset(&key, 0, sizeof(key));
        //    return SW_UNABLE_TO_PROCESS;
        //}
        //memset(&key, 0, sizeof(key));

        // レスポンスデータを生成
        rdata[0] = 0x7c;
        rdata[1] = 0x82;
        rdata[2] = HI(response_size + 4);
        rdata[3] = LO(response_size + 4);
        rdata[4] = TAG_RESPONSE;
        rdata[5] = 0x82;
        rdata[6] = HI(response_size);
        rdata[7] = LO(response_size);
        rapdu->len = response_size + 8;

    } else if (key_alg == ALG_ECC_256 || key_alg == ALG_ECC_384) {
        uint8_t key_size = (key_alg == ALG_ECC_256 ? ECC_256_PRI_KEY_SIZE : ECC_384_PRI_KEY_SIZE);
        if (input_size > key_size) {
            return SW_WRONG_DATA;
        }

        // 変数初期化
        uint8_t *output_data = rdata + 4;
        uint8_t key[key_size];
        uint8_t digest[key_size];
        uint8_t response_size = 0;

        // 該当のスロットから、EC秘密鍵を読込み、応答チャレンジを生成
        //if (read_file(key_path, key, 0, sizeof(key)) < 0) {
        //    return SW_UNABLE_TO_PROCESS;
        //}

        // 入力チャレンジがキー長より短い場合は、先頭から 0 を埋める
        uint8_t offset = key_size - input_size;
        memset(digest, 0, sizeof(digest));
        memcpy(digest + offset, input_data, input_size);

        // ECDSA署名を生成
        //ECC_Curve curve = (key_alg == ALG_ECC_256 ? ECC_SECP256R1 : ECC_SECP384R1);
        //if (ecdsa_sign(curve, key, digest, output_data) < 0) {
        //    memset(key, 0, sizeof(key));
        //    return SW_UNABLE_TO_PROCESS;
        //}
        //memset(key, 0, sizeof(key));
        //response_size = ecdsa_sig2ansi(key_size, output_data, output_data);

        // レスポンスデータを生成
        rdata[0] = 0x7c;
        rdata[1] = response_size + 2;
        rdata[2] = TAG_RESPONSE;
        rdata[3] = response_size;
        rapdu->len = response_size + 4;

    } else {
        return SW_SECURITY_STATUS_NOT_SATISFIED;
    }

    // 正常終了
    return SW_NO_ERROR;
}
