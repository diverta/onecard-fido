/* 
 * File:   ctap2_client_pin_token.h
 * Author: makmorit
 *
 * Created on 2019/02/23, 11:22
 */
#ifndef CTAP2_CLIENT_PIN_TOKEN_H
#define CTAP2_CLIENT_PIN_TOKEN_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void     ctap2_client_pin_token_init(bool force);
uint8_t *ctap2_client_pin_token_encoded(void);
size_t   ctap2_client_pin_token_encoded_size(void);
uint8_t  ctap2_client_pin_token_encode(uint8_t *p_key);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_CLIENT_PIN_TOKEN_H */
