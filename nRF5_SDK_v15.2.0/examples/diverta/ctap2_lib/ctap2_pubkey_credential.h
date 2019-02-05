/* 
 * File:   ctap2_pubkey_credential.h
 * Author: makmorit
 *
 * Created on 2019/01/08, 11:24
 */
#ifndef CTAP2_PUBKEY_CREDENTIAL_H
#define CTAP2_PUBKEY_CREDENTIAL_H

#include "ctap2_common.h"

#ifdef __cplusplus
extern "C" {
#endif

void     ctap2_pubkey_credential_generate_source(CTAP_PUBKEY_CRED_PARAM_T *param, CTAP_USER_ENTITY_T *user);
void     ctap2_pubkey_credential_generate_id(void);
uint8_t  ctap2_pubkey_credential_restore_private_key(CTAP_ALLOW_LIST_T *allowList);
uint8_t  ctap2_pubkey_credential_number(void);
uint8_t *ctap2_pubkey_credential_private_key(void);
CTAP_CREDENTIAL_DESC_T *ctap2_pubkey_credential_restored_id(void);

uint8_t *ctap2_pubkey_credential_source_hash(void);
size_t   ctap2_pubkey_credential_source_hash_size(void);

uint8_t *ctap2_pubkey_credential_id(void);
size_t   ctap2_pubkey_credential_id_size(void);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_PUBKEY_CREDENTIAL_H */

