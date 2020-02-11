/* 
 * File:   fido_flash_common.h
 * Author: makmorit
 *
 * Created on 2019/06/19, 10:10
 */
#ifndef FIDO_FLASH_COMMON_H
#define FIDO_FLASH_COMMON_H

#include "fds.h"

#ifdef __cplusplus
extern "C" {
#endif

void fido_flash_fds_force_gc(void);
bool fido_flash_fds_record_get(fds_record_desc_t *record_desc, uint32_t *record_buffer);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_FLASH_COMMON_H */
