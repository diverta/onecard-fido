/* 
 * File:   fido_status_indicator.h
 * Author: makmorit
 *
 * Created on 2022/12/30, 16:22
 */
#ifndef FIDO_STATUS_INDICATOR_H
#define FIDO_STATUS_INDICATOR_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// LED操作関数群
//
void        fido_status_indicator_none(void);
void        fido_status_indicator_idle(void);
void        fido_status_indicator_busy(void);
void        fido_status_indicator_prompt_reset(void);
void        fido_status_indicator_prompt_tup(void);
void        fido_status_indicator_pairing_mode(void);
void        fido_status_indicator_pairing_fail(bool short_interval);
void        fido_status_indicator_abort(void);
void        fido_status_indicator_ble_scanning(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_STATUS_INDICATOR_H */
