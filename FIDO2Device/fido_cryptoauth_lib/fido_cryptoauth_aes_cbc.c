/* 
 * File:   fido_cryptoauth_aes_cbc.c
 * Author: makmorit
 *
 * Created on 2020/02/03, 9:33
 */
//
// プラットフォーム非依存コード
//
#include "fido_cryptoauth.h"
#include "fido_cryptoauth_aes_cbc.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// ATECCx08A関連
//
#include "cryptoauthlib.h"

// for debug hex dump data
#define LOG_HEXDUMP_DEBUG_PASSWORD_NEW  false
#define LOG_HEXDUMP_DEBUG_PASSWORD_READ false

// パスワードのサイズ（32バイト）
#define AES_PASSWORD_SIZE ATCA_BLOCK_SIZE

// パスワードを保持（32バイト）
static uint8_t m_password[AES_PASSWORD_SIZE];

// イニシャルベクターを保持（16バイト）
static uint8_t m_initial_vector[AES_DATA_SIZE];

static bool aes_128_write_password(void)
{
    // 初期化
    if (fido_cryptoauth_init()== false) {
        return false;
    }

    // AESパスワードを、8番スロットの33バイト目(2ブロック目)以降に格納
    uint16_t key_id = KEY_ID_FOR_CONFIDENTIAL_DATA;
    ATCA_STATUS status = atcab_write_zone(ATCA_ZONE_DATA, key_id, 
        BLOCK_IDX_FOR_AES_PASSWORD, 0, m_password, AES_PASSWORD_SIZE);
    if (status != ATCA_SUCCESS) {
        fido_log_error("aes_128_write_password failed: atcab_write_zone(%d) returns 0x%02x",
            key_id, status);
        return false;
    }

    return true;
}

bool fido_cryptoauth_aes_cbc_new_password(void)
{
    // AESパスワード生成（32バイトのランダムベクターを作成）
    fido_cryptoauth_generate_random_vector(m_password, sizeof(m_password));

    // AESパスワードを、8番スロットの33バイト目(2ブロック目)以降に格納
    if (aes_128_write_password() == false) {
        return false;
    }

#if LOG_HEXDUMP_DEBUG_PASSWORD_NEW
    fido_log_debug("fido_cryptoauth_aes_cbc_new_password (%d bytes):", sizeof(m_password));
    fido_log_print_hexdump_debug(m_password, sizeof(m_password));
#endif

    return true;
}

bool fido_cryptoauth_aes_cbc_erase_password(void)
{
    // AESパスワードを 0xff で初期化
    memset(m_password, 0xff, sizeof(m_password));

    // AESパスワードを、8番スロットの33バイト目(2ブロック目)以降に格納
    if (aes_128_write_password() == false) {
        return false;
    }

    return true;
}

static bool aes_128_read_password(void)
{
    // 初期化
    memset(m_password, 0x00, sizeof(m_password));
    if (fido_cryptoauth_init()== false) {
        return false;
    }

    // AESパスワードを、8番スロットの33バイト目(2ブロック目)以降から読出し
    uint16_t key_id = KEY_ID_FOR_CONFIDENTIAL_DATA;
    ATCA_STATUS status = atcab_read_zone(ATCA_ZONE_DATA, key_id, 
        BLOCK_IDX_FOR_AES_PASSWORD, 0, m_password, AES_PASSWORD_SIZE);
    if (status != ATCA_SUCCESS) {
        fido_log_error("aes_128_read_password failed: atcab_read_zone(%d) returns 0x%02x", 
            key_id, status);
        return false;
    }

#if LOG_HEXDUMP_DEBUG_PASSWORD_READ
    fido_log_debug("aes_128_read_password (%d bytes):", sizeof(m_password));
    fido_log_print_hexdump_debug(m_password, sizeof(m_password));
#endif

    return true;
}

bool fido_cryptoauth_aes_cbc_check_password_exist(bool *exist)
{
    // パスワードを読出し
    if (aes_128_read_password()== false) {
        return false;
    }

    // パスワードが初期値（0xff）の場合は、exist に false を設定
    *exist = false;
    for (size_t i = 0; i < AES_PASSWORD_SIZE; i++) {
        if (m_password[i] != 0xff) {
            *exist = true;
            break;
        }
    }
    return true;
}

static bool aes_128_init_context(atca_aes_cbc_ctx_t *ctx, uint8_t *password)
{
    // Load AES keys into TempKey
    uint8_t target = NONCE_MODE_TARGET_TEMPKEY;
    ATCA_STATUS status = atcab_nonce_load(target, password, AES_PASSWORD_SIZE);
    if (status != ATCA_SUCCESS) {
        fido_log_error("aes_128_init_context failed: atcab_nonce_load(%d) returns 0x%02x", 
            target, status);
        return false;
    }

    // Init CBC mode context using 1st key in TempKey
    uint16_t key_id = ATCA_TEMPKEY_KEYID;
    memset(m_initial_vector, 0, sizeof(m_initial_vector));
    status = atcab_aes_cbc_init(ctx, key_id, 0, m_initial_vector);
    if (status != ATCA_SUCCESS) {
        fido_log_error("aes_128_init_context failed: atcab_aes_cbc_init(%d) returns 0x%04x", 
            key_id, status);
        return false;
    }

    return true;
}

static bool aes_128_encrypt(atca_aes_cbc_ctx_t *ctx, uint8_t *plaintext, uint8_t *encrypted, size_t *encrypt_size)
{
    // 暗号化対象のバイト数を退避
    size_t data_size = *encrypt_size;

    // Encrypt blocks
    size_t data_index;
    for (data_index = 0; data_index < data_size; data_index += AES_DATA_SIZE) {
        ATCA_STATUS status = atcab_aes_cbc_encrypt_block(ctx, plaintext + data_index, encrypted + data_index);
        if (status != ATCA_SUCCESS) {
            fido_log_error("aes_128_encrypt failed: atcab_aes_cbc_encrypt_block(%d) returns 0x%04x", 
                ctx->key_id, status);
            return false;
        }
    }

    // 実際に暗号化されたバイト数を設定
    *encrypt_size = data_index;
    return true;   
}

static bool aes_128_decrypt(atca_aes_cbc_ctx_t *ctx, uint8_t *encrypted, uint8_t *decrypted, size_t *decrypt_size)
{
    // 暗号化対象のバイト数を退避
    size_t data_size = *decrypt_size;

    // Decrypt blocks
    size_t data_index;
    for (data_index = 0; data_index < data_size; data_index += AES_DATA_SIZE) {
        ATCA_STATUS status = atcab_aes_cbc_decrypt_block(ctx, encrypted + data_index, decrypted + data_index);
        if (status != ATCA_SUCCESS) {
            fido_log_error("aes_128_decrypt failed: atcab_aes_cbc_decrypt_block(%d) returns 0x%04x", 
                ctx->key_id, status);
            return false;
        }
    }

    // 実際に暗号化されたバイト数を設定
    *decrypt_size = data_index;
    return true;   
}

bool fido_cryptoauth_aes_cbc_encrypt(uint8_t *plaintext, uint8_t *encrypted, size_t *encrypt_size)
{
    // AESパスワードを読込
    if (aes_128_read_password() == false) {
        return false;
    }

    // AESコンテキストを初期化
    atca_aes_cbc_ctx_t ctx;
    if (aes_128_init_context(&ctx, m_password) == false) {
        return false;
    }

    // AESパスワードで暗号化
    if (aes_128_encrypt(&ctx, plaintext, encrypted, encrypt_size) == false) {
        return false;
    }

    return true;   
}

bool fido_cryptoauth_aes_cbc_decrypt(uint8_t *encrypted, uint8_t *decrypted, size_t *decrypt_size)
{
    // AESパスワードを読込
    if (aes_128_read_password() == false) {
        return false;
    }

    // AESコンテキストを初期化
    atca_aes_cbc_ctx_t ctx;
    if (aes_128_init_context(&ctx, m_password) == false) {
        return false;
    }

    // AESパスワードで復号化
    if (aes_128_decrypt(&ctx, encrypted, decrypted, decrypt_size) == false) {
        return false;
    }

    return true;   
}
