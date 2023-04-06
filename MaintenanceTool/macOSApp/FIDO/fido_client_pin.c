//
//  fido_client_pin.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/04/06.
//
#include <stdlib.h>
#include "fido_crypto.h"
#include "fido_client_pin.h"
#include "FIDODefines.h"

// ヘルスチェック実行用のテストデータ
static const char *challenge = "This is challenge";

bool fido_client_pin_generate_pinauth(char *new_pin, char *old_pin, bool change_pin)
{
    // pinHashEncを生成
    if (change_pin) {
        if (generate_pin_hash_enc(old_pin) != CTAP1_ERR_SUCCESS) {
            return false;
        }
    }
    // newPinEncを生成
    if (generate_new_pin_enc(new_pin) != CTAP1_ERR_SUCCESS) {
        return false;
    }
    // pinAuthを生成
    if (generate_pin_auth(change_pin) != CTAP1_ERR_SUCCESS) {
        return false;
    }
    // 処理成功
    return true;
}

bool fido_client_pin_generate_pinauth_from_pintoken(uint8_t *pin_token)
{
    // clientDataHashを生成
    if (generate_client_data_hash(challenge) != CTAP1_ERR_SUCCESS) {
        return false;
    }
    // pinAuthを生成
    if (generate_pin_auth_from_client_data(pin_token, client_data_hash()) != CTAP1_ERR_SUCCESS) {
        return false;
    }
    // 処理成功
    return true;
}

bool fido_client_pin_generate_salt_auth(uint8_t *hmac_secret_salt)
{
    // saltEncを生成
    // Encrypt two salts (Called salt1 (32 bytes) and salt2 (32 bytes)) using sharedSecret
    // AES256-CBC(sharedSecret, IV=0, salt1 (32 bytes) || salt2 (32 bytes))
    if (generate_salt_enc(hmac_secret_salt, 64) != CTAP1_ERR_SUCCESS) {
        return false;
    }
    // saltAuthを生成
    // LEFT(HMAC-SHA-256(sharedSecret, saltEnc), 16)
    if (generate_salt_auth(salt_enc(), 64) != CTAP1_ERR_SUCCESS) {
        return false;
    }
    // 処理成功
    return true;
}
