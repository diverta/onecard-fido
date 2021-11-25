/* 
 * File:   ctap2_client_pin.h
 * Author: makmorit
 *
 * Created on 2019/02/18, 11:05
 */
#ifndef CTAP2_CLIENT_PIN_H
#define CTAP2_CLIENT_PIN_H

#ifdef __cplusplus
extern "C" {
#endif

uint8_t ctap2_client_pin_decode_request(uint8_t *cbor_data_buffer, size_t cbor_data_length);
void    ctap2_client_pin_perform_subcommand(uint8_t *response_buffer, size_t response_buffer_size);
void    ctap2_client_pin_send_response(void);
void    ctap2_client_pin_response_sent(void);
void    ctap2_client_pin_init(void);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_CLIENT_PIN_H */

