/* 
 * File:   fido_maintenance.h
 * Author: makmorit
 *
 * Created on 2019/03/26, 13:35
 */
#ifndef FIDO_MAINTENANCE_H
#define FIDO_MAINTENANCE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint8_t     fido_maintenance_command_byte(void);
uint8_t    *fido_maintenance_data_buffer(void);
size_t      fido_maintenance_data_buffer_size(void);
void        fido_maintenance_send_command_status(uint8_t ctap2_status);
void        fido_maintenance_command_ble(void);
void        fido_maintenance_command_hid(void);
void        fido_maintenance_command_report_sent(void);
void        fido_maintenance_command_flash_failed(void);
void        fido_maintenance_command_flash_gc_done(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_MAINTENANCE_H */
