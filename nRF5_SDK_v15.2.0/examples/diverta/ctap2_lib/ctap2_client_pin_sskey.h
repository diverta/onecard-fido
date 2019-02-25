/* 
 * File:   ctap2_client_pin_sskey.h
 * Author: makmorit
 *
 * Created on 2019/02/23, 11:17
 */
#ifndef CTAP2_CLIENT_PIN_SSKEY_H
#define CTAP2_CLIENT_PIN_SSKEY_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void     ctap2_client_pin_sskey_init(bool force);
uint8_t  ctap2_client_pin_sskey_generate(uint8_t *client_public_key_raw_data);
uint8_t *ctap2_client_pin_sskey_public_key(void);
uint8_t *ctap2_client_pin_sskey_calculate_hmac(uint8_t *src_data_1, size_t src_data_1_size, uint8_t *src_data_2, size_t src_data_2_size);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_CLIENT_PIN_SSKEY_H */

