/* 
 * File:   fido_flash_event.h
 * Author: makmorit
 *
 * Created on 2019/06/19, 10:10
 */
#ifndef FIDO_FLASH_EVENT_H
#define FIDO_FLASH_EVENT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// FDSイベントの読替用構造体
//
typedef struct {
    bool result;
    bool gc;
    bool delete_file;
    bool write_update;
    bool retry_counter_write;
} fido_flash_event_t;

void fido_command_fds_register(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_FLASH_EVENT_H */
