/* 
 * File:   ctap2_client_pin_crypto.c
 * Author: makmorit
 *
 * Created on 2019/02/25, 15:11
 */
#include "sdk_common.h"

// for nrf_crypto_aes_crypt
#include "nrf_crypto.h"
#include "nrf_crypto_init.h"
#include "nrf_crypto_error.h"
#include "app_error.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_client_pin_crypto
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug cbor data
#define NRF_LOG_DEBUG_DECRYPTED_DATA    false

// AESで使用する作業用エリア
static nrf_crypto_aes_info_t const *p_cbc_info;
static nrf_crypto_aes_context_t     cbc_decr_ctx;
static uint8_t                      iv[16];

static void app_error_check(char *function, ret_code_t err_code)
{
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("%s returns 0x%04x(%s)", 
            function, err_code, nrf_crypto_error_string_get(err_code));
        APP_ERROR_CHECK(err_code);
    }
}

size_t ctap2_client_pin_decrypt(uint8_t *p_key, uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted) 
{
#if NRF_LOG_DEBUG_DECRYPTED_DATA
    NRF_LOG_DEBUG("Encrypted data(%dbytes):", encrypted_size);
    NRF_LOG_HEXDUMP_DEBUG(p_encrypted, encrypted_size);
#endif

    ret_code_t err_code;
    size_t     decrypted_size;

#if NRF_MODULE_ENABLED(NRF_CRYPTO_BACKEND_MBEDTLS_AES_CBC)
    p_cbc_info = &g_nrf_crypto_aes_cbc_256_info;
#endif

    err_code = nrf_crypto_init();
    app_error_check("nrf_crypto_init", err_code);

    err_code = nrf_mem_init();
    app_error_check("nrf_mem_init", err_code);

    memset(iv, 0, sizeof(iv));
    decrypted_size = encrypted_size;
    err_code = nrf_crypto_aes_crypt(&cbc_decr_ctx, p_cbc_info, NRF_CRYPTO_DECRYPT, 
        p_key, iv, p_encrypted, encrypted_size, decrypted, &decrypted_size);
    app_error_check("nrf_crypto_aes_crypt", err_code);

#if NRF_LOG_DEBUG_DECRYPTED_DATA
    NRF_LOG_DEBUG("Decrypted data(%dbytes):", decrypted_size);
    NRF_LOG_HEXDUMP_DEBUG(decrypted, decrypted_size);
#endif

    return decrypted_size;
}

size_t ctap2_client_pin_encrypt(uint8_t *p_key, uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted) 
{
    ret_code_t err_code;
    size_t     encrypted_size;

#if NRF_MODULE_ENABLED(NRF_CRYPTO_BACKEND_MBEDTLS_AES_CBC)
    p_cbc_info = &g_nrf_crypto_aes_cbc_256_info;
#endif

    err_code = nrf_crypto_init();
    app_error_check("nrf_crypto_init", err_code);

    err_code = nrf_mem_init();
    app_error_check("nrf_mem_init", err_code);

    memset(iv, 0, sizeof(iv));
    encrypted_size = plaintext_size;
    err_code = nrf_crypto_aes_crypt(&cbc_decr_ctx, p_cbc_info, NRF_CRYPTO_ENCRYPT, 
        p_key, iv, p_plaintext, plaintext_size, encrypted, &encrypted_size);
    app_error_check("nrf_crypto_aes_crypt", err_code);

    return encrypted_size;
}
