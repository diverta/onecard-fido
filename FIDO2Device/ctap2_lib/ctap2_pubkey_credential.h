/* 
 * File:   ctap2_pubkey_credential.h
 * Author: makmorit
 *
 * Created on 2019/01/08, 11:24
 */
#ifndef CTAP2_PUBKEY_CREDENTIAL_H
#define CTAP2_PUBKEY_CREDENTIAL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void     ctap2_pubkey_credential_generate_source(void *ctap_pubkey_cred_param, void *ctap_user_entity);
void     ctap2_pubkey_credential_generate_id(void);
uint8_t  ctap2_pubkey_credential_restore_private_key(void *ctap_allow_list);
uint8_t  ctap2_pubkey_credential_number(void);
uint8_t *ctap2_pubkey_credential_private_key(void);
uint8_t *ctap2_pubkey_credential_cred_random(void);
void    *ctap2_pubkey_credential_restored_id(void);
uint8_t *ctap2_pubkey_credential_ble_auth_scan_param(void);

uint8_t *ctap2_pubkey_credential_source_hash(void);
size_t   ctap2_pubkey_credential_source_hash_size(void);

uint8_t *ctap2_pubkey_credential_id(void);
size_t   ctap2_pubkey_credential_id_size(void);

void     ctap2_pubkey_credential_do_sign(void);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_PUBKEY_CREDENTIAL_H */

