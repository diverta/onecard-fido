/* 
 * File:   fido_maintenance_attestation.h
 * Author: makmorit
 *
 * Created on 2023/01/13, 9:27
 */
#ifndef FIDO_MAINTENANCE_ATTESTATION_H
#define FIDO_MAINTENANCE_ATTESTATION_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        fido_maintenance_attestation_install(void);
void        fido_maintenance_attestation_install_retry(void);
void        fido_maintenance_attestation_reset(void);
void        fido_maintenance_attestation_record_updated(void);
void        fido_maintenance_attestation_aes_password_record_updated(void);
void        fido_maintenance_attestation_file_deleted(void);
void        fido_maintenance_attestation_token_counter_file_deleted(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_MAINTENANCE_ATTESTATION_H */
