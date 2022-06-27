/* 
 * File:   fido_development.h
 * Author: makmorit
 *
 * Created on 2022/06/27, 10:33
 */
#ifndef FIDO_DEVELOPMENT_H
#define FIDO_DEVELOPMENT_H

#include "fido_common.h"

#ifdef __cplusplus
extern "C" {
#endif

void        fido_development_command(TRANSPORT_TYPE transport_type);
void        fido_development_command_report_sent(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_DEVELOPMENT_H */
