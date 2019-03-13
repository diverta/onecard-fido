/* 
 * File:   ctap2_client_pin_store.h
 * Author: makmorit
 *
 * Created on 2019/02/27, 10:43
 */
#ifndef CTAP2_CLIENT_PIN_STORE_H
#define CTAP2_CLIENT_PIN_STORE_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool      ctap2_client_pin_store_hash_read(void);
bool      ctap2_client_pin_store_hash_write(uint8_t *p_pin_code_hash, uint32_t retry_counter);
uint8_t  *ctap2_client_pin_store_pin_code_hash(void);
uint32_t  ctap2_client_pin_store_retry_counter(void);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_CLIENT_PIN_STORE_H */
