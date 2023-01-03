/* 
 * File:   ccid_piv_authenticate.h
 * Author: makmorit
 *
 * Created on 2020/07/23, 16:32
 */
#ifndef CCID_PIV_AUTHENTICATE_H
#define CCID_PIV_AUTHENTICATE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void     ccid_piv_authenticate_reset_context(void);
uint16_t ccid_piv_authenticate_internal(void *p_capdu, void *p_rapdu, void *data_obj_ber_tlv_info);
uint16_t ccid_piv_authenticate_ecdh_with_kmk(void *p_capdu, void *p_rapdu, void *data_obj_ber_tlv_info);
uint16_t ccid_piv_authenticate_mutual_request(void *p_capdu, void *p_rapdu, void *data_obj_ber_tlv_info);
uint16_t ccid_piv_authenticate_mutual_response(void *p_capdu, void *p_rapdu, void *data_obj_ber_tlv_info);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_AUTHENTICATE_H */
