/* 
 * File:   fido_lock_channel.h
 * Author: makmorit
 *
 * Created on 2019/02/06, 13:28
 */
#ifndef FIDO_LOCK_CHANNEL_H
#define FIDO_LOCK_CHANNEL_H

#ifdef __cplusplus
extern "C" {
#endif

void     fido_lock_channel_start(uint32_t cid, uint8_t lock_param);
void     fido_lock_channel_cancel(void);
uint32_t fido_lock_channel_cid(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_LOCK_CHANNEL_H */
