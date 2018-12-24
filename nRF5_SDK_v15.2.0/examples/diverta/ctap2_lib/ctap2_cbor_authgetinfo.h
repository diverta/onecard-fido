/* 
 * File:   ctap2_cbor_authgetinfo.h
 * Author: makmorit
 *
 * Created on 2018/12/24, 9:32
 */
#ifndef CTAP2_CBOR_AUTHGETINFO_H
#define CTAP2_CBOR_AUTHGETINFO_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool ctap2_cbor_authgetinfo_response_message(uint8_t *response_buffer, size_t *response_length);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_CBOR_AUTHGETINFO_H */
