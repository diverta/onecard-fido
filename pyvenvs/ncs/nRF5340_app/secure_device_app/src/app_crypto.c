/* 
 * File:   app_crypto.c
 * Author: makmorit
 *
 * Created on 2021/05/12, 9:59
 */
#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/entropy.h>
#include <errno.h>
#include <init.h>

// for Mbed TLS
#include <mbedtls/aes.h>
#include <mbedtls/cipher.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/platform.h>

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_crypto);

#define LOG_DEBUG_AES_PLAINTEXT_DATA    false
#define LOG_DEBUG_AES_ENCRYPTED_DATA    false
#define LOG_DEBUG_AES_DECRYPTED_DATA    false
#define LOG_DEBUG_RANDOM_VECTOR_DATA    false

#define AES_KEY_SIZE 32

// 作業領域
static uint8_t m_aes_iv[16];

//
// AES-256暗号化処理
//
static bool aes_cbc_256_init(mbedtls_cipher_context_t *p_ctx)
{
    mbedtls_cipher_init(p_ctx);
    const mbedtls_cipher_info_t *p_cipher_info = mbedtls_cipher_info_from_values(MBEDTLS_CIPHER_ID_AES, AES_KEY_SIZE * 8, MBEDTLS_MODE_CBC);
    if (p_cipher_info == NULL) {
        LOG_ERR("mbedtls_cipher_info_from_values returns NULL");
        return false;
    }

    int ret = mbedtls_cipher_setup(p_ctx, p_cipher_info);
    if (ret != 0) {
        LOG_ERR("mbedtls_cipher_setup returns %d", ret);
        return false;
    }

    return true;
}

static bool aes_cbc_256_prepare(mbedtls_cipher_context_t *p_ctx, uint8_t *p_key, mbedtls_operation_t operation, mbedtls_cipher_padding_t padding)
{
    int ret = mbedtls_cipher_setkey(p_ctx, p_key, AES_KEY_SIZE * 8, operation);
    if (ret != 0) {
        LOG_ERR("mbedtls_cipher_setkey returns %d", ret);
        return false;
    }

    ret = mbedtls_cipher_set_padding_mode(p_ctx, padding);
    if (ret != 0) {
        LOG_ERR("mbedtls_cipher_set_padding_mode returns %d", ret);
        return false;
    }

    return true;
}

bool app_crypto_aes_cbc_256_encrypt(uint8_t *p_key, uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted, size_t *encrypted_size) 
{
#if LOG_DEBUG_AES_PLAINTEXT_DATA
    LOG_DBG("%d bytes", plaintext_size);
    LOG_HEXDUMP_DBG(p_plaintext, plaintext_size, "Plaintext data");
#endif

    mbedtls_cipher_context_t ctx;
    if (aes_cbc_256_init(&ctx) == false) {
        return false;
    }
    if (aes_cbc_256_prepare(&ctx, p_key, MBEDTLS_ENCRYPT, MBEDTLS_PADDING_NONE) == false) {
        return false;
    }

    memset(m_aes_iv, 0, sizeof(m_aes_iv));
    int ret = mbedtls_cipher_crypt(&ctx, m_aes_iv, sizeof(m_aes_iv), p_plaintext, plaintext_size, encrypted, encrypted_size);
    if (ret != 0) {
        LOG_ERR("mbedtls_cipher_crypt returns %d", ret);
        return false;
    }

    return true;
}

bool app_crypto_aes_cbc_256_decrypt(uint8_t *p_key, uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted, size_t *decrypted_size) 
{
#if LOG_DEBUG_AES_ENCRYPTED_DATA
    LOG_DBG("%d bytes", encrypted_size);
    LOG_HEXDUMP_DBG(p_encrypted, encrypted_size, "Encrypted data");
#endif

    mbedtls_cipher_context_t ctx;
    if (aes_cbc_256_init(&ctx) == false) {
        return false;
    }
    if (aes_cbc_256_prepare(&ctx, p_key, MBEDTLS_DECRYPT, MBEDTLS_PADDING_NONE) == false) {
        return false;
    }

    memset(m_aes_iv, 0, sizeof(m_aes_iv));
    int ret = mbedtls_cipher_crypt(&ctx, m_aes_iv, sizeof(m_aes_iv), p_encrypted, encrypted_size, decrypted, decrypted_size);
    if (ret != 0) {
        LOG_ERR("mbedtls_cipher_crypt returns %d", ret);
        return false;
    }

#if LOG_DEBUG_AES_DECRYPTED_DATA
    LOG_DBG("%d bytes", *decrypted_size);
    LOG_HEXDUMP_DBG(decrypted, *decrypted_size, "Decrypted data");
#endif

    return true;
}

//
// ランダムベクター生成
//
static mbedtls_ctr_drbg_context m_drbg_ctx;

bool app_crypto_generate_random_vector(uint8_t *vector_buf, size_t vector_size)
{
    // ランダムベクターを生成
    int ret = mbedtls_ctr_drbg_random(&m_drbg_ctx, vector_buf, vector_size);
    if (ret != 0) {
        LOG_ERR("mbedtls_ctr_drbg_random returns %d", ret);
        return false;
    }

#if LOG_DEBUG_RANDOM_VECTOR_DATA
    LOG_DBG("%d bytes", vector_size);
    LOG_HEXDUMP_DBG(vector_buf, vector_size, "Random vector data");
#endif

    return true;
}

//
// 初期設定
//   スタックを相当量消費するため、SYS_INITで実行します。
//
static const unsigned char ncs_seed[] = {0xde, 0xad, 0xbe, 0xef};

static int entropy_func(void *ctx, unsigned char *buf, size_t len)
{
    return entropy_get_entropy(ctx, buf, len);
}

static int app_crypto_init(const struct device *dev)
{
    // Get device binding named 'CRYPTOCELL'
    (void)dev;
    const char *name = DT_LABEL(DT_CHOSEN(zephyr_entropy));
    const struct device *p_device = device_get_binding(name);
    if (p_device == NULL) {
        LOG_ERR("device_get_binding(%s) returns NULL", name);
        return -ENODEV;
    }

    // Initialize random seed for CTR-DRBG
    mbedtls_ctr_drbg_init(&m_drbg_ctx);
    int ret = mbedtls_ctr_drbg_seed(&m_drbg_ctx, entropy_func, (void *)p_device, ncs_seed, sizeof(ncs_seed));
    if (ret != 0) {
        LOG_ERR("mbedtls_ctr_drbg_seed returns %d", ret);
        return -ENOTSUP;
    }

    LOG_INF("Mbed TLS random seed initialized");
    return 0;
}

SYS_INIT(app_crypto_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
