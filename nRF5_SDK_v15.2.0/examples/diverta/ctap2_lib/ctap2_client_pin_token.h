/* 
 * File:   ctap2_client_pin_token.h
 * Author: makmorit
 *
 * Created on 2019/02/23, 11:22
 */
#ifndef CTAP2_CLIENT_PIN_TOKEN_H
#define CTAP2_CLIENT_PIN_TOKEN_H

#ifdef __cplusplus
extern "C" {
#endif

void     ctap2_client_pin_token_init(void);
uint8_t *ctap2_client_pin_token_read(void);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_CLIENT_PIN_TOKEN_H */
