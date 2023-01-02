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

void fido_maintenance_command_ble(void);
void fido_maintenance_command_hid(void);
void fido_maintenance_command_report_sent(void);
void fido_maintenance_command_flash_failed(void);
void fido_maintenance_command_flash_gc_done(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_MAINTENANCE_H */
