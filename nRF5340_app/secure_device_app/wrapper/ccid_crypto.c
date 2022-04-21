/* 
 * File:   ccid_crypto.c
 * Author: makmorit
 *
 * Created on 2022/04/19, 10:12
 */
#include "app_crypto_rsa.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_crypto);
#endif

uint8_t *ccid_crypto_rsa_e_bytes(void)
{
    return app_crypto_rsa_e_bytes();
}

uint8_t ccid_crypto_rsa_e_size(void)
{
    return app_crypto_rsa_e_size();
}

bool ccid_crypto_rsa_private(uint8_t *rsa_private_key_raw, uint8_t *input, uint8_t *output)
{
    return app_crypto_rsa_private(rsa_private_key_raw, input, output);
}

bool ccid_crypto_rsa_import(uint8_t *rsa_private_key_raw, uint8_t *rsa_public_key_raw, unsigned int nbits)
{
    if (nbits != 2048) {
        return false;
    }
    return app_crypto_rsa_import_pubkey_from_prvkey(rsa_private_key_raw, rsa_public_key_raw);
}

bool ccid_crypto_rsa_generate_key(uint8_t *rsa_private_key_raw, uint8_t *rsa_public_key_raw, unsigned int nbits)
{
    return false;
}
