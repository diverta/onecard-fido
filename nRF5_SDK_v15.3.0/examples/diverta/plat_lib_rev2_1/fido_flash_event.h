/* 
 * File:   fido_flash_event.h
 * Author: makmorit
 *
 * Created on 2019/06/19, 10:10
 */
#ifndef FIDO_FLASH_EVENT_H
#define FIDO_FLASH_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

void fido_flash_event_set_gc_forced(void);
void fido_flash_fds_event_register(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_FLASH_EVENT_H */
