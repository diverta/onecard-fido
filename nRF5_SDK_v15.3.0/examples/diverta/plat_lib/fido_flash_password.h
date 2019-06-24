/* 
 * File:   fido_flash_password.h
 * Author: makmorit
 *
 * Created on 2018/12/27, 14:48
 */
#ifndef FIDO_FLASH_PASSWORD_H
#define FIDO_FLASH_PASSWORD_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool     fido_flash_password_generate(void);
uint8_t *fido_flash_password_get(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_FLASH_PASSWORD_H */
