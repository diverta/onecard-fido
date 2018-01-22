#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_U2F)
#include <stdio.h>
#include <string.h>
#include "ble_u2f.h"

// for logging informations
#define NRF_LOG_MODULE_NAME "ble_u2f_crypto"
#include "nrf_log.h"

// for generate pkey
#include "nrf_crypto_init.h"
#include "nrf_crypto_keys.h"
#include "nrf_crypto_hash.h"
#include "nrf_crypto_ecdsa.h"


// micro-eccで生成される鍵情報
NRF_CRYPTO_ECC_PRIVATE_KEY_CREATE(private_key, SECP256R1);
NRF_CRYPTO_ECC_PUBLIC_KEY_CREATE(public_key, SECP256R1);

// micro-eccで生成される署名情報
NRF_CRYPTO_ECC_PRIVATE_KEY_RAW_CREATE(private_key_for_sign, SECP256R1);
NRF_CRYPTO_HASH_CREATE(hashed_buffer, SHA256);
NRF_CRYPTO_ECDSA_SIGNATURE_CREATE(signature, SECP256R1);

// ハッシュ化データ、署名データに関する情報
const nrf_crypto_hash_info_t hash_info_sha256 =
{
    .hash_type = NRF_CRYPTO_HASH_TYPE_SHA256,
    .endian_type = NRF_CRYPTO_ENDIAN_LE
};
const nrf_crypto_signature_info_t sig_info_p256 =
{
    .curve_type     = NRF_CRYPTO_CURVE_SECP256R1,
    .hash_type      = NRF_CRYPTO_HASH_TYPE_SHA256,
    .endian_type    = NRF_CRYPTO_ENDIAN_LE
};

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
    // micro-eccの初期化が未実行の場合は
    // nrf_crypto_initを実行する
    err_code = nrf_crypto_init();
    NRF_LOG_DEBUG("nrf_crypto_init() returns 0x%02x \r\n", err_code);
    if (err_code != NRF_ERROR_MODULE_ALREADY_INITIALIZED) {
        APP_ERROR_CHECK(err_code);
    }
}

void ble_u2f_crypto_generate_keypair(void)
{
    ret_code_t err_code;
    NRF_LOG_DEBUG("ble_u2f_crypto_generate_keypair start \r\n");

    // micro-eccの初期化を実行する
    ble_u2f_crypto_init();

    // 秘密鍵および公開鍵を生成する
    err_code = nrf_crypto_ecc_key_pair_generate(BLE_LESC_CURVE_TYPE_INFO, &private_key, &public_key);
    NRF_LOG_DEBUG("ble_u2f_crypto_generate_keypair: nrf_crypto_ecc_key_pair_generate() returns 0x%02x \r\n", err_code);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEBUG("ble_u2f_crypto_generate_keypair end \r\n");
}

nrf_value_length_t *ble_u2f_crypto_private_key(void)
{
    return &private_key;
}

nrf_value_length_t *ble_u2f_crypto_public_key(void)
{
    return &public_key;
}


uint32_t ble_u2f_crypto_sign(uint8_t *private_key_le, uint8_t *signature_base_buffer, uint16_t signature_base_buffer_length)
{
    ret_code_t err_code;
    NRF_LOG_DEBUG("ble_u2f_crypto_sign start \r\n");

    // micro-eccの初期化を実行する
    ble_u2f_crypto_init();

    // 署名対象バイト配列からSHA256アルゴリズムにより、
    // ハッシュデータ作成
    err_code = nrf_crypto_hash_compute(hash_info_sha256, 
                    signature_base_buffer, signature_base_buffer_length, 
                    &hashed_buffer);
    NRF_LOG_DEBUG("ble_u2f_crypto_sign: nrf_crypto_hash_compute() returns 0x%02x \r\n", err_code);
    APP_ERROR_CHECK(err_code);

    // 署名用の秘密鍵（32バイト）を設定
    //   private_key_leは
    //   リトルエンディアン化された秘密鍵のバイト配列
    private_key_for_sign.p_value = private_key_le;
    private_key_for_sign.length = NRF_CRYPTO_ECC_PRIVATE_KEY_SIZE_SECP256R1;

    // ハッシュデータと秘密鍵により、署名データ作成
    err_code = nrf_crypto_ecdsa_sign_hash(sig_info_p256, &private_key_for_sign, &hashed_buffer, &signature);
    NRF_LOG_DEBUG("ble_u2f_crypto_sign: nrf_crypto_ecdsa_sign_hash() returns 0x%02x \r\n", err_code);

    NRF_LOG_DEBUG("ble_u2f_crypto_sign end \r\n");
    return NRF_SUCCESS;
}

bool ble_u2f_crypto_create_asn1_signature(nrf_value_length_t *p_signature)
{
    // 格納領域を確保
    uint8_t *asn1_signature = p_signature->p_value;
    if (asn1_signature == NULL) {
        NRF_LOG_DEBUG("ble_u2f_crypto_create_asn1_signature: allocation failed \r\n");
        return false;
    }
    NRF_LOG_DEBUG("ble_u2f_crypto_create_asn1_signature start \r\n");

    // 格納領域を初期化
    memset(asn1_signature, 0, ASN1_SIGNATURE_MAXLEN);

    // 署名データのr部、s部の先頭バイトが
    // 0b80 以上であれば、
    // その直前に 0x00 を挿入する必要がある
    // (MSBを参照して判定)
    int part_length = 32;
    uint8_t *rbytes = signature.p_value;
    uint8_t *sbytes = signature.p_value + part_length;
    
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
    // エンディアンを変換
    for (int j = part_length; j > 0; j--) {
        asn1_signature[i++] = rbytes[j-1];
    }

    // 署名データのs部を格納
    asn1_signature[i++] = ASN_INT;
    asn1_signature[i++] = sbytes_leading + part_length;
    if (sbytes_leading == 1) {
        asn1_signature[i++] = 0x00;
    }
    // エンディアンを変換
    for (int k = part_length; k > 0; k--) {
        asn1_signature[i++] = sbytes[k-1];
    }

    // 生成されたASN.1形式署名の
    // サイズを構造体に設定
    p_signature->length = i;

    NRF_LOG_DEBUG("ble_u2f_crypto_create_asn1_signature end \r\n");
    return true;
}

#endif // NRF_MODULE_ENABLED(BLE_U2F)
