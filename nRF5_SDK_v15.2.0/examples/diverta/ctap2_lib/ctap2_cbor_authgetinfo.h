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

uint8_t ctap2_cbor_authgetinfo_encode_request(uint8_t *encoded_buff, size_t *encoded_buff_size);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_CBOR_AUTHGETINFO_H */
