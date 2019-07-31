/* 
 * File:   fido_status_indicator.h
 * Author: makmorit
 *
 * Created on 2019/07/31, 09:27
 */
#ifndef FIDO_STATUS_INDICATOR_H
#define FIDO_STATUS_INDICATOR_H

void fido_processing_led_timedout_handler(void);
void fido_idling_led_timedout_handler(void);

//
// C --> CPP 呼出用インターフェース
//
void _fido_status_indicator_none(void);
void _fido_status_indicator_idle(void);
void _fido_status_indicator_prompt_reset(void);
void _fido_status_indicator_prompt_tup(void);
void _fido_status_indicator_pairing_mode(void);
void _fido_status_indicator_pairing_fail(void);
void _fido_status_indicator_abort(void);

#endif /* FIDO_STATUS_INDICATOR_H */

