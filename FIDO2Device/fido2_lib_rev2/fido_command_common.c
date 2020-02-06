/* 
 * File:   fido_command_common.c
 * Author: makmorit
 *
 * Created on 2020/02/06, 12:21
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// CTAP2、U2Fで共用する各種処理
//
bool fido_command_check_skey_cert_exist(void)
{
    if (fido_flash_skey_cert_read() == false) {
        // 秘密鍵と証明書をFlash ROMから読込
        // NGであれば、エラーレスポンスを生成して戻す
        fido_log_error("Private key and certification read error");
        return false;
    }

    if (fido_flash_skey_cert_available() == false) {
        // 秘密鍵と証明書がFlash ROMに登録されていない場合
        // エラーレスポンスを生成して戻す
        return false;
    }
    
    // 秘密鍵と証明書が登録されている場合はtrue
    return true;
}

bool fido_command_check_aes_password_exist(void)
{
    if (fido_flash_password_get() == NULL) {
        // キーハンドルを暗号化するために必要な
        // AESパスワードが生成されていない場合
        // エラーレスポンスを生成して戻す
        return false;
    }

    // AESパスワードが生成されている場合はtrue
    return true;
}

size_t fido_command_aes_cbc_decrypt(uint8_t *p_encrypted, size_t encrypted_size, uint8_t *decrypted)
{
    return fido_crypto_aes_cbc_256_decrypt(fido_flash_password_get(), p_encrypted, encrypted_size, decrypted);
}

size_t fido_command_aes_cbc_encrypt(uint8_t *p_plaintext, size_t plaintext_size, uint8_t *encrypted)
{
    return fido_crypto_aes_cbc_256_encrypt(fido_flash_password_get(), p_plaintext, plaintext_size, encrypted);
}

void fido_command_sskey_init(bool force) 
{
    fido_crypto_sskey_init(force);
}

uint8_t *fido_command_keypair_public_key(void)
{
    return fido_crypto_keypair_public_key();
}
