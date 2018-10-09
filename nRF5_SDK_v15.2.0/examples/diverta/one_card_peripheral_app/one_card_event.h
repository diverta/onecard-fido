/* 
 * File:   one_card_event.h
 * Author: makmorit
 *
 * Created on 2018/10/09, 11:59
 */

#ifndef ONE_CARD_EVENT_H
#define ONE_CARD_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

void one_card_ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context);

#ifdef __cplusplus
}
#endif

#endif /* ONE_CARD_EVENT_H */

