/* 
 * File:   ccid_piv.h
 * Author: makmorit
 *
 * Created on 2020/06/01, 9:55
 */
#ifndef CCID_PIV_H
#define CCID_PIV_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PIV_INS_SELECT                  0xA4
#define PIV_INS_GET_DATA                0xCB
#define PIV_INS_VERIFY                  0x20
#define PIV_INS_CHANGE_REFERENCE_DATA   0x24
#define PIV_INS_RESET_RETRY_COUNTER     0x2C
#define PIV_INS_GENERAL_AUTHENTICATE    0x87
#define PIV_INS_PUT_DATA                0xDB
#define PIV_INS_GENERATE_ASYMMETRIC_KEY_PAIR 0x47

#define PIV_INS_SET_MANAGEMENT_KEY      0xFF
#define PIV_INS_RESET                   0xFB
#define PIV_INS_IMPORT_ASYMMETRIC_KEY   0xFE
#define PIV_INS_GET_VERSION             0xFD
#define PIV_INS_GET_SERIAL              0xF8

//
// 関数群
//
uint8_t *ccid_piv_rid(void);
size_t   ccid_piv_rid_size(void);

void ccid_piv_apdu_process(command_apdu_t *capdu, response_apdu_t *rapdu);
void ccid_piv_stop_applet(void);
bool ccid_piv_admin_mode_get(void);
void ccid_piv_admin_mode_set(bool mode);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_H */
