#include <stdio.h>
#include <string.h>

#include "fido_command_common.h"
#include "u2f.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// キーハンドル生成・格納用領域
// Register, Authenticateで共通使用
uint8_t keyhandle_base_buffer[64];
uint8_t keyhandle_buffer[64];

void u2f_keyhandle_generate(uint8_t *p_appid_hash)
{
    // nrf_cc310により、キーペアを新規生成する
    fido_crypto_keypair_generate();
    uint8_t *private_key_value = fido_crypto_keypair_private_key();
    uint32_t private_key_length = fido_crypto_keypair_private_key_size();

    // Register/Authenticateリクエストから取得した
    // appIdHash、秘密鍵を指定の領域に格納
    memset(keyhandle_base_buffer, 0, sizeof(keyhandle_base_buffer));
    memcpy(keyhandle_base_buffer, p_appid_hash, U2F_APPID_SIZE);
    memcpy(keyhandle_base_buffer + U2F_APPID_SIZE, private_key_value, private_key_length);

    // AES暗号化対象のバイト配列＆長さを指定し、
    // Cipher Feedback Modeによる暗号化を実行
    memset(keyhandle_buffer, 0, sizeof(keyhandle_buffer));
    uint16_t data_length = 64;
    fido_command_aes_cbc_encrypt(keyhandle_base_buffer, data_length, keyhandle_buffer);
}


void u2f_keyhandle_restore(uint8_t *keyhandle_value, uint32_t keyhandle_length)
{
    // Authenticateリクエストから取得した
    // キーハンドルを指定の領域に格納
    memset(keyhandle_buffer, 0, sizeof(keyhandle_buffer));
    memcpy(keyhandle_buffer, keyhandle_value, keyhandle_length);

    // Cipher Feedback ModeによりAES暗号化された
    // バイト配列を、同じ手法により復号化
    memset(keyhandle_base_buffer, 0, sizeof(keyhandle_base_buffer));
    uint16_t data_length = 64;
    fido_command_aes_cbc_decrypt(keyhandle_buffer, data_length, keyhandle_base_buffer);
 }
