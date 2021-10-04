/* 
 * File:   ctap2_extension_hmac_secret.c
 * Author: makmorit
 *
 * Created on 2019/05/20, 10:30
 */
#include "ctap2_cbor.h"
#include "fido_command_common.h"
#include "fido_common.h"
#include "ctap2_common.h"
#include "ctap2_cbor_parse.h"
#include "ctap2_pubkey_credential.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ctap2_extension_hmac_secret);
#endif

// for debug cbor data
#define LOG_DEBUG_SALT_BUFF false
#define LOG_DEBUG_CBOR_BUFF false

// 作業用の領域
static uint8_t salt[64];
static uint8_t output[64];
static uint8_t encrypted_output[64];
static uint8_t hmac[HMAC_SHA_256_SIZE];

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
    fido_command_sskey_calculate_hmac_sha256(
        ext->hmac_secret.saltEnc, ext->hmac_secret.saltLen, NULL, 0, hmac);

    // クライアントから受信したsaltAuth（16バイト）を、
    // saltEncから生成されたHMAC SHA-256ハッシュと比較し、
    // 異なる場合はエラーを戻す
    if (memcmp(hmac, ext->hmac_secret.saltAuth, 16) != 0) {
        return CTAP2_ERR_EXTENSION_FIRST;
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
#if LOG_DEBUG_SALT_BUFF
    fido_log_debug("Encrypted salt (%d bytes):", ext->hmac_secret.saltLen);
    fido_log_print_hexdump_debug(ext->hmac_secret.saltEnc, ext->hmac_secret.saltLen);
#endif

    // 変数初期化
    memset(extension_cbor, 0x00, sizeof(extension_cbor));
    extension_cbor_size = 0;

    // CTAP2クライアントから受け取った公開鍵と、
    // 鍵交換用キーペアの秘密鍵を使用し、共通鍵ハッシュを生成
    // （ClientPIN1における共通鍵ハッシュ生成処理と同様の処理）
    uint8_t ret = fido_command_sskey_generate((uint8_t *)&ext->hmac_secret.keyAgreement.key);
    if (ret != CTAP1_ERR_SUCCESS) {
        fido_log_error("generate sharedSecret from keyAgreement failed");
        return ret;
    }

    // CTAP2クライアントから受け取ったsaltAuthを、
    // 共通鍵ハッシュを使用して検証
    ret = verify_salt_auth(ext);
    if (ret != CTAP1_ERR_SUCCESS) {
        fido_log_error("saltAuth verification failed");
        return ret;
    }

    // CTAP2クライアントから受け取ったsaltEncを、
    // 共通鍵ハッシュを使用して復号化
    size_t salt_size = fido_command_sskey_aes_256_cbc_decrypt(
        ext->hmac_secret.saltEnc, ext->hmac_secret.saltLen, salt);
    if (salt_size != ext->hmac_secret.saltLen) {
        fido_log_error("saltEnc decrpytion failed");
        return CTAP1_ERR_OTHER;
    }

    // CredRandomとsaltから、outputを計算
    uint8_t *key_data = ctap2_pubkey_credential_cred_random();
    size_t   key_size = CRED_RANDOM_SIZE;
    fido_command_calc_hash_hmac_sha256(key_data, key_size, salt, 32, NULL, 0, output);
    if (salt_size == 64) {
        fido_command_calc_hash_hmac_sha256(key_data, key_size, salt + 32, 32, NULL, 0, output + 32);
    }

    // 計算されたoutputを、共通鍵ハッシュを使用して暗号化
    size_t encrypted_size = fido_command_sskey_aes_256_cbc_encrypt(
        output, salt_size, encrypted_output);
    if (encrypted_size != salt_size) {
        fido_log_error("output encrpytion failed");
        return CTAP1_ERR_OTHER;
    }

#if LOG_DEBUG_SALT_BUFF
    fido_log_debug("credRandom(%d bytes):", CRED_RANDOM_SIZE);
    fido_log_print_hexdump_debug(ctap2_pubkey_credential_cred_random(), CRED_RANDOM_SIZE);
    fido_log_debug("Decrypted salt(%d bytes):", salt_size);
    fido_log_print_hexdump_debug(salt, salt_size);
    fido_log_debug("Calculated output(%d bytes):", salt_size);
    fido_log_print_hexdump_debug(output, salt_size);
    fido_log_debug("Encrypted output(%d bytes):", salt_size);
    fido_log_print_hexdump_debug(encrypted_output, salt_size);
#endif

    // レスポンスCBORを生成
    ret = generate_cbor_for_get(encrypted_output, encrypted_size);
    if (ret != CTAP1_ERR_SUCCESS) {
        fido_log_error("generate response CBOR failed");
        return ret;
    }

#if LOG_DEBUG_CBOR_BUFF
    fido_log_debug("Response CBOR(%d bytes):", extension_cbor_size);
    fido_log_print_hexdump_debug(extension_cbor, extension_cbor_size);
#endif

    return CTAP1_ERR_SUCCESS;
}
