/* 
 * File:   ctap2_extension_hmac_secret.c
 * Author: makmorit
 *
 * Created on 2019/05/20, 10:30
 */
#include "sdk_common.h"

#include "cbor.h"
#include "fido_common.h"
#include "ctap2_common.h"
#include "fido_aes_cbc_256_crypto.h"
#include "ctap2_client_pin_sskey.h"
#include "ctap2_cbor_parse.h"
#include "ctap2_pubkey_credential.h"

// for calculate hmac
#include "nrf_crypto_hmac.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_extension_hmac_secret
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug cbor data
#define NRF_LOG_DEBUG_SALT_BUFF false
#define NRF_LOG_DEBUG_CBOR_BUFF false
#define NRF_LOG_DEBUG_BUFF (NRF_LOG_DEBUG_SALT_BUFF || NRF_LOG_DEBUG_CBOR_BUFF)

// 作業用の領域
static uint8_t salt[64];
static uint8_t output[64];
static uint8_t encrypted_output[64];

// HMAC SHA-256ハッシュ格納領域
static nrf_crypto_hmac_context_t hmac_context;
static uint8_t                   hmac_data[NRF_CRYPTO_HASH_SIZE_SHA256];
static size_t                    hmac_data_size;

// 生成されたCBORを格納
static uint8_t extension_cbor[80];
static size_t  extension_cbor_size;

uint8_t *ctap2_extension_hmac_secret_cbor(void)
{
    return extension_cbor;
}

size_t ctap2_extension_hmac_secret_cbor_size(void)
{
    return extension_cbor_size;
}

#if NRF_LOG_DEBUG_BUFF
static void print_hexdump_debug(uint8_t *buff, size_t size)
{
    int j, k;
    for (j = 0; j < size; j += 64) {
        k = size - j;
        NRF_LOG_HEXDUMP_DEBUG(buff + j, (k < 64) ? k : 64);
    }
}
#endif

uint8_t ctap2_extension_hmac_secret_cbor_for_create(void)
{
    CborEncoder extensions;
    uint8_t     ret;

    cbor_encoder_init(&extensions, extension_cbor, sizeof(extension_cbor), 0);

    CborEncoder hmac_secret_map;
    ret = cbor_encoder_create_map(&extensions, &hmac_secret_map, 1);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    ret = cbor_encode_text_stringz(&hmac_secret_map, "hmac-secret");
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    ret = cbor_encode_boolean(&hmac_secret_map, 1);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    ret = cbor_encoder_close_container(&extensions, &hmac_secret_map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    extension_cbor_size = cbor_encoder_get_buffer_size(&extensions, extension_cbor);
    return CTAP1_ERR_SUCCESS;
}

uint8_t verify_salt_auth(CTAP_EXTENSIONS_T *ext)
{
    // 共通鍵ハッシュを利用し、
    // CTAP2クライアントから受領したsaltEncを
    // HMAC SHA-256アルゴリズムでハッシュ化
    uint8_t *hmac = ctap2_client_pin_sskey_calculate_hmac(
        ext->hmac_secret.saltEnc, ext->hmac_secret.saltLen, NULL, 0);
    if (hmac == NULL) {
        return CTAP1_ERR_OTHER;
    }

    // クライアントから受信したsaltAuth（16バイト）を、
    // saltEncから生成されたHMAC SHA-256ハッシュと比較し、
    // 異なる場合はエラーを戻す
    if (memcmp(hmac, ext->hmac_secret.saltAuth, 16) != 0) {
        return CTAP2_ERR_EXTENSION_FIRST;
    }

    return CTAP1_ERR_SUCCESS;
}

static uint8_t calculate_hmac_with_cred_random(uint8_t *src_data, size_t src_data_size, uint8_t *dest_data)
{
    // HMACハッシュ計算には、CredRandomを使用
    ret_code_t err_code = nrf_crypto_hmac_init(
        &hmac_context, &g_nrf_crypto_hmac_sha256_info, 
        ctap2_pubkey_credential_cred_random(), CRED_RANDOM_SIZE);
    if (err_code != CTAP1_ERR_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_hmac_init failed");
        return CTAP1_ERR_OTHER;
    }

    // 引数を計算対象に設定
    err_code = nrf_crypto_hmac_update(&hmac_context, src_data, src_data_size);
    if (err_code != CTAP1_ERR_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_hmac_update failed");
        return CTAP1_ERR_OTHER;
    }

    // HMACハッシュを計算
    hmac_data_size = sizeof(hmac_data);
    err_code = nrf_crypto_hmac_finalize(&hmac_context, hmac_data, &hmac_data_size);
    if (err_code != CTAP1_ERR_SUCCESS) {
        NRF_LOG_ERROR("nrf_crypto_hmac_finalize failed");
        return CTAP1_ERR_OTHER;
    }

    // 計算結果をdestDataへコピー
    for (uint8_t i = 0; i < sizeof(hmac_data); i++) {
        dest_data[i] = hmac_data[i];
    }

    return CTAP1_ERR_SUCCESS;
}

static uint8_t generate_cbor_for_get(uint8_t *encrypted_output, size_t encrypted_output_size)
{
    CborEncoder extensions;
    uint8_t     ret;

    cbor_encoder_init(&extensions, extension_cbor, sizeof(extension_cbor), 0);

    CborEncoder hmac_secret_map;
    ret = cbor_encoder_create_map(&extensions, &hmac_secret_map, 1);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    ret = cbor_encode_text_stringz(&hmac_secret_map, "hmac-secret");
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    // output1／output2を設定
    ret = cbor_encode_byte_string(&hmac_secret_map, encrypted_output, encrypted_output_size);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    ret = cbor_encoder_close_container(&extensions, &hmac_secret_map);
    if (ret != CborNoError) {
        return CTAP1_ERR_OTHER;
    }

    extension_cbor_size = cbor_encoder_get_buffer_size(&extensions, extension_cbor);
    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_extension_hmac_secret_cbor_for_get(CTAP_EXTENSIONS_T *ext)
{
#if NRF_LOG_DEBUG_SALT_BUFF
    NRF_LOG_DEBUG("Encrypted salt (%d bytes):", ext->hmac_secret.saltLen);
    NRF_LOG_HEXDUMP_DEBUG(ext->hmac_secret.saltEnc, ext->hmac_secret.saltLen);
#endif

    // 変数初期化
    memset(extension_cbor, 0x00, sizeof(extension_cbor));
    extension_cbor_size = 0;

    // CTAP2クライアントから受け取った公開鍵と、
    // 鍵交換用キーペアの秘密鍵を使用し、共通鍵ハッシュを生成
    // （ClientPIN1における共通鍵ハッシュ生成処理と同様の処理）
    uint8_t ret = ctap2_client_pin_sskey_generate((uint8_t *)&ext->hmac_secret.keyAgreement.key);
    if (ret != CTAP1_ERR_SUCCESS) {
        NRF_LOG_DEBUG("generate sharedSecret from keyAgreement failed");
        return ret;
    }

    // CTAP2クライアントから受け取ったsaltAuthを、
    // 共通鍵ハッシュを使用して検証
    ret = verify_salt_auth(ext);
    if (ret != CTAP1_ERR_SUCCESS) {
        NRF_LOG_ERROR("saltAuth verification failed");
        return ret;
    }

    // CTAP2クライアントから受け取ったsaltEncを、
    // 共通鍵ハッシュを使用して復号化
    size_t salt_size = fido_aes_cbc_256_decrypt(ctap2_client_pin_sskey_hash(), 
        ext->hmac_secret.saltEnc, ext->hmac_secret.saltLen, salt);
    if (salt_size != ext->hmac_secret.saltLen) {
        NRF_LOG_ERROR("saltEnc decrpytion failed");
        return CTAP1_ERR_OTHER;
    }

    // CredRandomとsaltから、outputを計算
    ret = calculate_hmac_with_cred_random(salt, 32, output);
    if (ret != CTAP1_ERR_SUCCESS) {
        NRF_LOG_ERROR("output1 calculation failed");
        return ret;
    }
    if (salt_size == 64) {
        ret = calculate_hmac_with_cred_random(salt + 32, 32, output + 32);
        if (ret != CTAP1_ERR_SUCCESS) {
            NRF_LOG_ERROR("output2 calculation failed");
            return ret;
        }
    }

    // 計算されたoutputを、共通鍵ハッシュを使用して暗号化
    size_t encrypted_size = fido_aes_cbc_256_encrypt(ctap2_client_pin_sskey_hash(), 
        output, salt_size, encrypted_output);
    if (encrypted_size != salt_size) {
        NRF_LOG_ERROR("output encrpytion failed");
        return CTAP1_ERR_OTHER;
    }

#if NRF_LOG_DEBUG_SALT_BUFF
    NRF_LOG_DEBUG("credRandom(%d bytes):", CRED_RANDOM_SIZE);
    print_hexdump_debug(ctap2_pubkey_credential_cred_random(), CRED_RANDOM_SIZE);
    NRF_LOG_DEBUG("Decrypted salt(%d bytes):", salt_size);
    print_hexdump_debug(salt, salt_size);
    NRF_LOG_DEBUG("Calculated output(%d bytes):", salt_size);
    print_hexdump_debug(output, salt_size);
    NRF_LOG_DEBUG("Encrypted output(%d bytes):", salt_size);
    print_hexdump_debug(encrypted_output, salt_size);
#endif

    // レスポンスCBORを生成
    ret = generate_cbor_for_get(encrypted_output, encrypted_size);
    if (ret != CTAP1_ERR_SUCCESS) {
        NRF_LOG_ERROR("generate response CBOR failed");
        return ret;
    }

#if NRF_LOG_DEBUG_CBOR_BUFF
    NRF_LOG_DEBUG("Response CBOR(%d bytes):", extension_cbor_size);
    print_hexdump_debug(extension_cbor, extension_cbor_size);
#endif

    return CTAP1_ERR_SUCCESS;
}
