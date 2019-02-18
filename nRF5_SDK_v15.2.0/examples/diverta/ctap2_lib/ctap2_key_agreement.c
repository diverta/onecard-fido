/* 
 * File:   ctap2_key_agreement.c
 * Author: makmorit
 *
 * Created on 2019/02/18, 15:31
 */
#include "sdk_common.h"
#include <stdio.h>
#include <string.h>

#include "fido_common.h"
#include "ctap2_cbor_parse.h"

// for keypair informations
#include "fido_crypto_keypair.h"
#include "nrf_crypto_ecdsa.h"
#include "app_error.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_key_agreement
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// 鍵ペア情報
//   この領域に格納される鍵は
//   ビッグエンディアン配列となる
static uint8_t private_key[NRF_CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE];
static uint8_t public_key[NRF_CRYPTO_ECC_SECP256R1_RAW_PUBLIC_KEY_SIZE];
static size_t  private_key_size = 0;
static size_t  public_key_size = 0;

// keyAgreementKey (CBOR) を保持
static uint8_t encoded_cose_key[256];
static size_t  encoded_cose_key_size;

void ctap2_key_agreement_generate_keypair(void)
{
    if (public_key_size == 0 && public_key_size == 0) {
        // キーペアを新規生成し、内部領域に格納
        fido_crypto_keypair_generate();
        private_key_size = fido_crypto_keypair_private_key_size();
        public_key_size = fido_crypto_keypair_public_key_size();
        memcpy(private_key, fido_crypto_keypair_private_key(), private_key_size);
        memcpy(public_key, fido_crypto_keypair_public_key(), public_key_size);
    }
}

uint8_t ctap2_key_agreement_encode_cose_key(void)
{
    // CBORエンコーダー初期化
    CborEncoder encoder;
    memset(encoded_cose_key, 0x00, sizeof(encoded_cose_key));
    cbor_encoder_init(&encoder, encoded_cose_key, sizeof(encoded_cose_key), 0);

    // CBORエンコード実行
    uint8_t *x = public_key;
    uint8_t *y = public_key + 32;
    int32_t alg = COSE_ALG_ES256;
    uint8_t ret = encode_cose_pubkey(&encoder, x, y, alg);
    if (ret != CborNoError) {
        return CTAP2_ERR_PROCESSING;
    }

    // CBORエンコードデータ長を取得
    encoded_cose_key_size = cbor_encoder_get_buffer_size(&encoder, encoded_cose_key);
    return CTAP1_ERR_SUCCESS;
}