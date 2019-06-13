/* 
 * File:   fido_user_presence.h
 * Author: makmorit
 *
 * Created on 2019/01/15, 10:25
 */
#ifndef FIDO_USER_PRESENCE_H
#define FIDO_USER_PRESENCE_H

#ifdef __cplusplus
extern "C" {
#endif

// キープアライブ・タイマー
#define U2F_KEEPALIVE_INTERVAL_MSEC   500
#define CTAP2_KEEPALIVE_INTERVAL_MSEC 200

void    fido_user_presence_terminate(void);
void    fido_user_presence_verify_start(uint32_t timeout_msec, void *p_context);
uint8_t fido_user_presence_verify_end(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_USER_PRESENCE_H */

