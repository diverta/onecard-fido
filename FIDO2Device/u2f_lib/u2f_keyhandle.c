#include <stdio.h>
#include <string.h>

#include "fido_command_common.h"
#include "fido_common.h"
#include "u2f.h"
#include "u2f_signature.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// キーハンドル生成・格納用領域
// Register, Authenticateで共通使用
//   keyhandle_base_buffer
//     暗号化される前（＝復合化された後）のキーハンドル
//     この中に、AppIDHash、秘密鍵を格納
//   keyhandle_buffer
//     暗号化された後のキーハンドル。
//     U2Fサーバー／クライアントとの受け渡しに使用
#define KEYHANDLE_MAX_SIZE 96
static uint8_t keyhandle_base_buffer[KEYHANDLE_MAX_SIZE];
static uint8_t keyhandle_buffer[KEYHANDLE_MAX_SIZE];
static size_t  keyhandle_buffer_block_size;

uint8_t *u2f_keyhandle_base_buffer(void)
{
    return keyhandle_base_buffer;
}

uint8_t *u2f_keyhandle_buffer(void)
{
    return keyhandle_buffer;
}

size_t u2f_keyhandle_buffer_size(void)
{
    return keyhandle_buffer_block_size;
}

void u2f_keyhandle_generate(uint8_t *p_appid_hash)
{
    // 新規生成したキーペアの秘密鍵を格納
    uint8_t *private_key_value = fido_command_keypair_privkey_for_keyhandle();
    uint32_t private_key_length = U2F_PRIVKEY_SIZE;

    // Register/Authenticateリクエストから取得した
    // appIdHash、秘密鍵を指定の領域に格納
    memset(keyhandle_base_buffer, 0, sizeof(keyhandle_base_buffer));
    memcpy(keyhandle_base_buffer, p_appid_hash, U2F_APPID_SIZE);
    memcpy(keyhandle_base_buffer + U2F_APPID_SIZE, private_key_value, private_key_length);
    size_t offset = U2F_APPID_SIZE + U2F_PRIVKEY_SIZE;

    // BLE近接認証機能用のスキャンパラメーターを末尾に追加
    //  先頭バイト: パラメーター長
    //  後続バイト: パラメーターのバイト配列を格納
    offset += ble_peripheral_auth_scan_param_prepare(keyhandle_base_buffer + offset);

    // 暗号化対象ブロックサイズを設定
    //   AESの仕様上、16の倍数でなければならない
    keyhandle_buffer_block_size = fido_calculate_aes_block_size(offset);

    // AES暗号化対象のバイト配列＆長さを指定し、暗号化を実行
    memset(keyhandle_buffer, 0, sizeof(keyhandle_buffer));
    fido_command_aes_cbc_encrypt(keyhandle_base_buffer, keyhandle_buffer_block_size, keyhandle_buffer);
}

void u2f_keyhandle_restore(uint8_t *keyhandle_value, uint32_t keyhandle_length)
{
    // Authenticateリクエストから取得した
    // キーハンドルを指定の領域に格納
    memset(keyhandle_buffer, 0, sizeof(keyhandle_buffer));
    memcpy(keyhandle_buffer, keyhandle_value, keyhandle_length);

    // AES暗号化されたバイト配列を復号化
    memset(keyhandle_base_buffer, 0, sizeof(keyhandle_base_buffer));
    fido_command_aes_cbc_decrypt(keyhandle_buffer, keyhandle_length, keyhandle_base_buffer);
}

uint8_t *u2f_keyhandle_ble_auth_scan_param(void)
{
    // BLEスキャンパラメーター格納領域の先頭を戻す
    return (keyhandle_base_buffer + U2F_APPID_SIZE + U2F_PRIVKEY_SIZE);
}
