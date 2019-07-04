/* 
 * File:   fido_maintenance.h
 * Author: makmorit
 *
 * Created on 2019/03/26, 13:35
 */
#ifndef FIDO_MAINTENANCE_H
#define FIDO_MAINTENANCE_H

// for Flash ROM event
#include "fido_flash_event.h"

#ifdef __cplusplus
extern "C" {
#endif

void fido_maintenance_command(void);
void fido_maintenance_command_send_response(void const *p_evt);
void fido_maintenance_command_report_sent(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_MAINTENANCE_H */
