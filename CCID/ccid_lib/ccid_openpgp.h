/* 
 * File:   ccid_openpgp.h
 * Author: makmorit
 *
 * Created on 2021/02/08, 15:43
 */
#ifndef CCID_OPENPGP_H
#define CCID_OPENPGP_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OPENPGP_INS_SELECT                  0xA4

//
// 関数群
//
bool ccid_openpgp_aid_is_applet(command_apdu_t *capdu);
void ccid_openpgp_apdu_process(command_apdu_t *capdu, response_apdu_t *rapdu);
void ccid_openpgp_stop_applet(void);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_H */
