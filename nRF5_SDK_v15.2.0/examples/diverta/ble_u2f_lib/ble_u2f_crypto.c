#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include "ble_u2f.h"
#include "ble_u2f_securekey.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_u2f_crypto
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for generate pkey
#include "nrf_crypto_init.h"
#include "nrf_crypto_hash.h"
#include "nrf_crypto_ecdsa.h"
#include "app_error.h"

// nrf_cc310で生成される鍵情報
static nrf_crypto_ecc_key_pair_generate_context_t keygen_context;
static nrf_crypto_ecc_private_key_t               private_key;
static nrf_crypto_ecc_public_key_t                public_key;

// ハッシュ化データ、署名データに関する情報
static nrf_crypto_hash_context_t       hash_context;
static nrf_crypto_hash_sha256_digest_t hash_digest;
static nrf_crypto_ecc_private_key_t    private_key_for_sign;
static nrf_crypto_ecdsa_sign_context_t sign_context;
static nrf_crypto_ecdsa_signature_t    signature;

// ASN.1形式に変換された署名を格納する領域の大きさ
#define ASN1_SIGNATURE_MAXLEN 72

#define ASN_INT 0x02;
#define ASN_SEQUENCE 0x30;


void ble_u2f_crypto_init(void)
{
    ret_code_t err_code;
    if (nrf_crypto_is_initialized() == true) {
        return;
    }
    // 初期化が未実行の場合は
    // nrf_crypto_initを実行する
    err_code = nrf_crypto_init();
    NRF_LOG_DEBUG("nrf_crypto_init() returns 0x%02x ", err_code);
    if (err_code != NRF_ERROR_MODULE_ALREADY_INITIALIZED) {
        APP_ERROR_CHECK(err_code);
    }
}

void ble_u2f_crypto_generate_keypair(void)
{
    ret_code_t err_code;
    NRF_LOG_DEBUG("ble_u2f_crypto_generate_keypair start ");

    // nrf_cryptoの初期化を実行する
    ble_u2f_crypto_init();

    // 秘密鍵および公開鍵を生成する
    err_code = nrf_crypto_ecc_key_pair_generate(
        &keygen_context, &g_nrf_crypto_ecc_secp256r1_curve_info, &private_key, &public_key);
    NRF_LOG_DEBUG("ble_u2f_crypto_generate_keypair: nrf_crypto_ecc_key_pair_generate() returns 0x%02x ", err_code);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEBUG("ble_u2f_crypto_generate_keypair end ");
}

void ble_u2f_crypto_private_key(uint8_t *p_raw_data, size_t *p_raw_data_size)
{
    // 秘密鍵データをビッグエンディアンでp_raw_data配列に格納
    *p_raw_data_size = NRF_CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE;
    ret_code_t err_code = nrf_crypto_ecc_private_key_to_raw(&private_key, p_raw_data, p_raw_data_size);
    NRF_LOG_DEBUG("nrf_crypto_ecc_private_key_to_raw() returns 0x%02x ", err_code);
    APP_ERROR_CHECK(err_code);
}

void ble_u2f_crypto_public_key(uint8_t *p_raw_data, size_t *p_raw_data_size)
{
    // 公開鍵データをビッグエンディアンでp_raw_data配列に格納
    *p_raw_data_size = NRF_CRYPTO_ECC_SECP256R1_RAW_PUBLIC_KEY_SIZE;
    ret_code_t err_code = nrf_crypto_ecc_public_key_to_raw(&public_key, p_raw_data, p_raw_data_size);
    NRF_LOG_DEBUG("nrf_crypto_ecc_public_key_to_raw() returns 0x%02x ", err_code);
    APP_ERROR_CHECK(err_code);
}

uint32_t ble_u2f_crypto_sign(uint8_t *private_key_be, ble_u2f_context_t *p_u2f_context)
{
    ret_code_t err_code;
    NRF_LOG_DEBUG("ble_u2f_crypto_sign start ");

    // nrf_cryptoの初期化を実行する
    ble_u2f_crypto_init();

    // 署名対象バイト配列からSHA256アルゴリズムにより、
    // ハッシュデータ作成
    uint8_t *signature_base_buffer = p_u2f_context->signature_data_buffer;
    uint16_t signature_base_buffer_length = p_u2f_context->signature_data_buffer_length;
    size_t digest_size = sizeof(hash_digest);
    err_code = nrf_crypto_hash_calculate(
        &hash_context, 
        &g_nrf_crypto_hash_sha256_info, 
        signature_base_buffer, 
        signature_base_buffer_length, 
        hash_digest, 
        &digest_size);
    NRF_LOG_DEBUG("ble_u2f_crypto_sign: nrf_crypto_hash_calculate() returns 0x%02x ", err_code);
    APP_ERROR_CHECK(err_code);

    // 署名に使用する秘密鍵（32バイト）を取得
    //   SDK 15以降はビッグエンディアンで引き渡す必要あり
    err_code = nrf_crypto_ecc_private_key_from_raw(
        &g_nrf_crypto_ecc_secp256r1_curve_info,
        &private_key_for_sign, 
        private_key_be, 
        NRF_CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE);
    NRF_LOG_DEBUG("ble_u2f_crypto_sign: nrf_crypto_ecc_private_key_from_raw() returns 0x%02x ", err_code);
    APP_ERROR_CHECK(err_code);

    // ハッシュデータと秘密鍵により、署名データ作成
    size_t signature_size = sizeof(signature);
    err_code = nrf_crypto_ecdsa_sign(
        &sign_context, 
        &private_key_for_sign,
        hash_digest,
        digest_size,
        signature, 
        &signature_size);
    NRF_LOG_DEBUG("ble_u2f_crypto_sign: nrf_crypto_ecdsa_sign() returns 0x%02x ", err_code);

    NRF_LOG_DEBUG("ble_u2f_crypto_sign end ");
    return NRF_SUCCESS;
}

bool ble_u2f_crypto_create_asn1_signature(ble_u2f_context_t *p_u2f_context)
{
    // 格納領域を確保
    uint8_t *asn1_signature = p_u2f_context->signature_data_buffer;
    if (asn1_signature == NULL) {
        NRF_LOG_DEBUG("ble_u2f_crypto_create_asn1_signature: allocation failed ");
        return false;
    }
    NRF_LOG_DEBUG("ble_u2f_crypto_create_asn1_signature start ");

    // 格納領域を初期化
    memset(asn1_signature, 0, ASN1_SIGNATURE_MAXLEN);

    // 署名データのr部、s部の先頭バイトが
    // 0b80 以上であれば、
    // その直前に 0x00 を挿入する必要がある
    // (MSBを参照して判定)
    int part_length = 32;
    uint8_t *p_signature_value = signature;
    uint8_t *rbytes = p_signature_value;
    uint8_t *sbytes = p_signature_value + part_length;
    
    int rbytes_leading = 0;
    if (rbytes[part_length-1] & 0x80) {
        rbytes_leading = 1;
    }

    int sbytes_leading = 0;
    if (sbytes[part_length-1] & 0x80) {
        sbytes_leading = 1;
    }

    // 署名データのヘッダー部を格納
    int total_length = 4 + rbytes_leading + part_length 
                         + sbytes_leading + part_length;
    int i = 0;
    asn1_signature[i++] = ASN_SEQUENCE;
    asn1_signature[i++] = total_length;

    // 署名データのr部を格納
    asn1_signature[i++] = ASN_INT;
    asn1_signature[i++] = rbytes_leading + part_length;
    if (rbytes_leading == 1) {
        asn1_signature[i++] = 0x00;
    }
    for (int j = 0; j < part_length; j++) {
        asn1_signature[i++] = rbytes[j];
    }

    // 署名データのs部を格納
    asn1_signature[i++] = ASN_INT;
    asn1_signature[i++] = sbytes_leading + part_length;
    if (sbytes_leading == 1) {
        asn1_signature[i++] = 0x00;
    }
    for (int k = 0; k < part_length; k++) {
        asn1_signature[i++] = sbytes[k];
    }

    // 生成されたASN.1形式署名の
    // サイズを構造体に設定
    p_u2f_context->signature_data_buffer_length = i;

    NRF_LOG_DEBUG("ble_u2f_crypto_create_asn1_signature end ");
    return true;
}
