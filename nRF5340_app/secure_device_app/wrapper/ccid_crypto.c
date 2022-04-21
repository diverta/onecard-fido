/* 
 * File:   ccid_crypto.c
 * Author: makmorit
 *
 * Created on 2022/04/19, 10:12
 */
#include "ccid_crypto.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_crypto);
#endif

//
// E は定数として、このモジュール内で管理
//
static uint8_t E[] = {0, 1, 0, 1};

uint8_t *ccid_crypto_rsa_e_bytes(void)
{
    return E;
}

uint8_t ccid_crypto_rsa_e_size(void)
{
    return sizeof(E);
}

bool ccid_crypto_rsa_private(uint8_t *rsa_private_key_raw, uint8_t *input, uint8_t *output)
{
    return false;
}

bool ccid_crypto_rsa_import(uint8_t *rsa_private_key_raw, uint8_t *rsa_public_key_raw, unsigned int nbits)
{
    return false;
}

bool ccid_crypto_rsa_generate_key(uint8_t *rsa_private_key_raw, uint8_t *rsa_public_key_raw, unsigned int nbits)
{
    return false;
}
