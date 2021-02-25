/* 
 * File:   ccid_flash_object.h
 * Author: makmorit
 *
 * Created on 2021/02/15, 11:58
 */
#ifndef CCID_FLASH_OBJECT_H
#define CCID_FLASH_OBJECT_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint8_t    *ccid_flash_object_read_buffer(void);
uint8_t    *ccid_flash_object_write_buffer(void);
size_t      ccid_flash_object_rw_buffer_size(void);

#ifdef __cplusplus
}
#endif

#endif /* CCID_FLASH_OBJECT_H */
