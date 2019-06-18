/* 
 * File:   fido_timer.h
 * Author: makmorit
 *
 * Created on 2019/06/18, 11:00
 */
#ifndef FIDO_TIMER_H
#define FIDO_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

void fido_comm_interval_timer_stop(void);
void fido_comm_interval_timer_start(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_TIMER_H */
