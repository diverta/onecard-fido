/* 
 * File:   demo_cdc_receive.h
 * Author: makmorit
 *
 * Created on 2019/10/16, 11:12
 */
#ifndef DEMO_CDC_RECEIVE_H
#define DEMO_CDC_RECEIVE_H

#ifdef __cplusplus
extern "C" {
#endif

void demo_cdc_receive_init(void);
void demo_cdc_receive_char(char c);
void demo_cdc_receive_char_terminate(void);
void demo_cdc_receive_on_request_received(void);

#ifdef __cplusplus
}
#endif

#endif /* DEMO_CDC_RECEIVE_H */
