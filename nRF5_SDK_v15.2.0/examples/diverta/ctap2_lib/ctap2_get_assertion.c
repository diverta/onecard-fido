/* 
 * File:   ctap2_get_assertion.c
 * Author: makmorit
 *
 * Created on 2019/01/03, 11:05
 */
#include "sdk_common.h"

#include "cbor.h"
#include "ctap2_common.h"
#include "ctap2_cbor_authgetinfo.h"
#include "ctap2_cbor_parse.h"
#include "fido_common.h"
#include "fido_crypto.h"
#include "fido_crypto_ecb.h"
#include "fido_crypto_keypair.h"

// for u2f_flash_keydata_read & u2f_flash_keydata_available
#include "u2f_flash.h"

// for u2f_crypto_sign & other
#include "u2f_crypto.h"

// for u2f_securekey_skey_be
#include "u2f_register.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ctap2_get_assertion
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

uint8_t ctap2_get_assertion_decode_request(uint8_t *cbor_data_buffer, size_t cbor_data_length)
{
    return CTAP1_ERR_SUCCESS;
}

bool ctap2_get_assertion_is_tup_needed(void)
{
    return true;
}

uint8_t ctap2_get_assertion_generate_response_items(void)
{
    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_get_assertion_encode_response(uint8_t *encoded_buff, size_t *encoded_buff_size)
{
    return CTAP1_ERR_SUCCESS;
}

uint8_t ctap2_get_assertion_update_token_counter(void)
{
    return CTAP1_ERR_SUCCESS;
}
