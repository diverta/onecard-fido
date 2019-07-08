/* 
 * File:   fido_maintenance.h
 * Author: makmorit
 *
 * Created on 2019/03/26, 13:35
 */
#ifndef FIDO_MAINTENANCE_H
#define FIDO_MAINTENANCE_H

#ifdef __cplusplus
extern "C" {
#endif

void fido_maintenance_command(void);
void fido_maintenance_command_report_sent(void);
void fido_maintenance_command_flash_failed(void);
void fido_maintenance_command_flash_gc_done(void);
void fido_maintenance_command_skey_cert_file_deleted(void);
void fido_maintenance_command_token_counter_file_deleted(void);
void fido_maintenance_command_aes_password_record_updated(void);
void fido_maintenance_command_skey_cert_record_updated(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_MAINTENANCE_H */
