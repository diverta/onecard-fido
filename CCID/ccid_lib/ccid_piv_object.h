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

bool ccid_piv_object_sn_get(uint8_t *buffer, size_t *size);
bool ccid_piv_object_chuid_get(uint8_t *buffer, size_t *size);
bool ccid_piv_object_ccc_get(uint8_t *buffer, size_t *size);
bool ccid_piv_object_cert_cauth_get(uint8_t *buffer, size_t *size);
bool ccid_piv_object_cert_pauth_get(uint8_t *buffer, size_t *size);
bool ccid_piv_object_cert_digsig_get(uint8_t *buffer, size_t *size);
bool ccid_piv_object_cert_keyman_get(uint8_t *buffer, size_t *size);
bool ccid_piv_object_key_history_get(uint8_t *buffer, size_t *size);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_OBJECT_H */
