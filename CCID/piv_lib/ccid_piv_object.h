/* 
 * File:   ccid_piv_object.h
 * Author: makmorit
 *
 * Created on 2020/06/02, 11:06
 */
#ifndef CCID_PIV_OBJECT_H
#define CCID_PIV_OBJECT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool    ccid_piv_object_chuid_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_ccc_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_read_private_key(uint8_t tag, uint8_t alg, uint8_t *buffer, size_t *size);
bool    ccid_piv_object_cert_pauth_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_cert_digsig_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_cert_keyman_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_card_admin_key_get(uint8_t *buffer, size_t *size, uint8_t *alg);
bool    ccid_piv_object_get(uint8_t data_obj_tag, uint8_t *buffer, size_t *size);
bool    ccid_piv_object_is_key_tag_exist(uint8_t key_tag);
bool    ccid_piv_object_is_obj_tag_exist(uint8_t obj_tag);
bool    ccid_piv_object_pin_get(uint8_t obj_tag, uint8_t *pin_code, uint8_t *retries);
bool    ccid_piv_object_pin_set(uint8_t obj_tag, uint8_t *pin_code, uint8_t retries);
void    ccid_piv_object_pin_set_retry(void);
void    ccid_piv_object_pin_set_resume(bool success);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_OBJECT_H */
