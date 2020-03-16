/* 
 * File:   fido_flash_pairing_mode.h
 * Author: makmorit
 *
 * Created on 2019/07/08, 9:32
 */
#ifndef FIDO_FLASH_PAIRING_MODE_H
#define FIDO_FLASH_PAIRING_MODE_H

#ifdef __cplusplus
extern "C" {
#endif

bool fido_flash_pairing_mode_flag(void);
void fido_flash_pairing_mode_flag_set(void);
void fido_flash_pairing_mode_flag_clear(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_FLASH_PAIRING_MODE_H */
