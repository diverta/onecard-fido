#include "sdk_common.h"

#include <stdio.h>
#include <string.h>

// for logging informations
#define NRF_LOG_MODULE_NAME u2f_crypto
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for generate pkey
#include "nrf_crypto_init.h"
#include "nrf_crypto_hash.h"
#include "nrf_crypto_ecdsa.h"
#include "app_error.h"

#include "fido_crypto.h"

// ハッシュ化データ、署名データに関する情報
static nrf_crypto_hash_sha256_digest_t hash_digest;
static nrf_crypto_ecc_private_key_t    private_key_for_sign;
static nrf_crypto_ecdsa_signature_t    signature;
static size_t signature_size;

// ASN.1形式に変換された署名を格納する領域の大きさ
#define ASN1_SIGNATURE_MAXLEN 72

#define ASN_INT 0x02;
#define ASN_SEQUENCE 0x30;

// 署名ベースおよび署名を編集するための作業領域（固定長）
#define SIGNATURE_BASE_BUFFER_LENGTH 384
static uint8_t signature_data_buffer[SIGNATURE_BASE_BUFFER_LENGTH];
static size_t  signature_data_size;

//
// For research 
//   署名検証不正の件に関する調査用
//   See: https://github.com/diverta/onecard-fido/pull/95
//
#define DEBUG_VERIFY_SIGN false
#if DEBUG_VERIFY_SIGN
static uint8_t pk[64] = {
    0x43, 0xa0, 0xd8, 0xad, 0x68, 0x27, 0x25, 0xc4, 0xec, 0x66, 0xd6, 0xb8, 0x3e, 0x11, 0xcd, 0x00, 
    0x3c, 0xed, 0x8f, 0x78, 0xd1, 0x48, 0xc7, 0x9d, 0xe4, 0x7f, 0xab, 0x53, 0x2e, 0x0e, 0xbb, 0x1e,
    0xf4, 0xb3, 0xa8, 0xed, 0x71, 0xd4, 0x2a, 0x39, 0x54, 0x9d, 0x92, 0x84, 0x8d, 0xb9, 0x7e, 0xdf, 
    0x85, 0xa0, 0x7b, 0xa2, 0x2a, 0x06, 0x93, 0xaa, 0x36, 0xa6, 0xba, 0x41, 0x75, 0x50, 0xed, 0xe8
};
static uint8_t public_key_be[64];
static nrf_crypto_ecc_public_key_t public_key_for_verify;
static nrf_crypto_ecdsa_verify_context_t verify_context = {0};
    
void verify_sign(void) 
{
    ret_code_t err_code;

    // 署名に使用する秘密鍵（32バイト）を取得
    //   SDK 15以降はビッグエンディアンで引き渡す必要あり
    for (int i = 0; i < 64; i++) {
        public_key_be[i] = pk[63 - i];
    }

    err_code = nrf_crypto_ecc_public_key_from_raw(
        &g_nrf_crypto_ecc_secp256r1_curve_info,
        &public_key_for_verify, 
        public_key_be, 
        NRF_CRYPTO_ECC_SECP256R1_RAW_PUBLIC_KEY_SIZE);
    NRF_LOG_DEBUG("u2f_crypto_sign: nrf_crypto_ecc_public_key_from_raw() returns 0x%02x ", err_code);
    APP_ERROR_CHECK(err_code);

    // for debug
    NRF_LOG_DEBUG("hash_digest: ");
    NRF_LOG_HEXDUMP_INFO(hash_digest, sizeof(hash_digest));

    // ハッシュデータと秘密鍵により、署名データ作成
    size_t digest_size = sizeof(hash_digest);
    err_code = nrf_crypto_ecdsa_verify(
        &verify_context, 
        &public_key_for_verify,
        hash_digest,
        digest_size,
        signature, 
        signature_size);
    NRF_LOG_DEBUG("u2f_crypto_sign: nrf_crypto_ecdsa_verify() returns 0x%02x ", err_code);
}
#endif

uint8_t *u2f_crypto_signature_data_buffer(void)
{
    return signature_data_buffer;
}

size_t u2f_crypto_signature_data_size(void)
{
    return signature_data_size;
}

void u2f_crypto_signature_data_size_set(size_t size)
{
    signature_data_size = size;
}

void u2f_crypto_init(void)
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

uint32_t u2f_crypto_sign(uint8_t *private_key_be)
{
    ret_code_t err_code;
    NRF_LOG_DEBUG("ECDSA sign start ");

    // nrf_cryptoの初期化を実行する
    u2f_crypto_init();

    // 署名対象バイト配列からSHA256アルゴリズムにより、
    // ハッシュデータ作成
    uint8_t *signature_base_buffer = signature_data_buffer;
    uint16_t signature_base_buffer_length = signature_data_size;

#if DEBUG_VERIFY_SIGN
    // for debug
    NRF_LOG_DEBUG("signature_base_buffer: %d bytes", signature_base_buffer_length);
    NRF_LOG_HEXDUMP_INFO(signature_base_buffer, signature_base_buffer_length);
#endif

    size_t digest_size = sizeof(hash_digest);
    fido_crypto_generate_sha256_hash(signature_base_buffer, signature_base_buffer_length, hash_digest, &digest_size);

    // 署名に使用する秘密鍵（32バイト）を取得
    //   SDK 15以降はビッグエンディアンで引き渡す必要あり
    err_code = nrf_crypto_ecc_private_key_from_raw(
        &g_nrf_crypto_ecc_secp256r1_curve_info,
        &private_key_for_sign, 
        private_key_be, 
        NRF_CRYPTO_ECC_SECP256R1_RAW_PRIVATE_KEY_SIZE);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_DEBUG("nrf_crypto_ecc_private_key_from_raw() returns 0x%02x ", err_code);
    }
    APP_ERROR_CHECK(err_code);

    // ハッシュデータと秘密鍵により、署名データ作成
    signature_size = sizeof(signature);
    fido_crypto_ecdsa_sign(&private_key_for_sign, hash_digest, digest_size, signature, &signature_size);

#if DEBUG_VERIFY_SIGN
    // 署名の妥当性検証を行う
    verify_sign();
#endif

    NRF_LOG_DEBUG("ECDSA sign end ");
    return NRF_SUCCESS;
}

bool u2f_crypto_create_asn1_signature(void)
{
    // 格納領域を確保
    uint8_t *asn1_signature = signature_data_buffer;
    if (asn1_signature == NULL) {
        NRF_LOG_DEBUG("u2f_crypto_create_asn1_signature: allocation failed ");
        return false;
    }
    NRF_LOG_DEBUG("Create ASN.1 signature start ");

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
    if (rbytes[0] & 0x80) {
        rbytes_leading = 1;
    }

    int sbytes_leading = 0;
    if (sbytes[0] & 0x80) {
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
    signature_data_size = i;

    NRF_LOG_DEBUG("Create ASN.1 signature end ");
    return true;
}
