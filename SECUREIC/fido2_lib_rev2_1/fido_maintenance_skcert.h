/* 
 * File:   fido_maintenance_skcert.h
 * Author: makmorit
 *
 * Created on 2020/02/06, 10:41
 */
#ifndef FIDO_MAINTENANCE_SKCERT_H
#define FIDO_MAINTENANCE_SKCERT_H

#ifdef __cplusplus
extern "C" {
#endif

void fido_maintenance_command_skey_cert(void);
void fido_maintenance_command_skey_cert_report_sent(void);

void fido_maintenance_command_flash_failed(void);
void fido_maintenance_command_flash_gc_done(void);
void fido_maintenance_command_skey_cert_file_deleted(void);
void fido_maintenance_command_token_counter_file_deleted(void);
void fido_maintenance_command_skey_cert_record_updated(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_MAINTENANCE_SKCERT_H */
