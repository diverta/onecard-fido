/* 
 * File:   fido_crypto_hmac_sha1.c
 * Author: makmorit
 *
 * Created on 2022/11/24, 14:44
 */
#include "sdk_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_crypto_hmac_sha1
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

void fido_crypto_hmac_sha1_calculate(
    uint8_t *key_data, size_t key_data_size, 
    uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size,
    uint8_t *dest_data)
{
    // TODO: 仮の実装です。
    NRF_LOG_ERROR("fido_crypto_hmac_sha1_calculate not implemented");
}
