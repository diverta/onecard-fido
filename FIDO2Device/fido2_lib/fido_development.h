/* 
 * File:   fido_development.h
 * Author: makmorit
 *
 * Created on 2022/06/27, 10:33
 */
#ifndef FIDO_DEVELOPMENT_H
#define FIDO_DEVELOPMENT_H

#ifdef __cplusplus
extern "C" {
#endif

void        fido_development_command_hid(void);
void        fido_development_command_report_sent(void);
void        fido_development_command_attestation_record_updated(void);
void        fido_development_command_aes_password_record_updated(void);
void        fido_development_command_attestation_file_deleted(void);
void        fido_development_command_token_counter_file_deleted(void);
void        fido_development_command_flash_failed(void);
void        fido_development_command_flash_gc_done(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_DEVELOPMENT_H */
