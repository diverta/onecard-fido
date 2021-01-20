/* 
 * File:   fido_status_indicator.h
 * Author: makmorit
 *
 * Created on 2019/07/16, 16:42
 */
#ifndef FIDO_STATUS_INDICATOR_H
#define FIDO_STATUS_INDICATOR_H

#ifdef __cplusplus
extern "C" {
#endif

void fido_processing_led_timedout_handler(void);
void fido_idling_led_timedout_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_STATUS_INDICATOR_H */
