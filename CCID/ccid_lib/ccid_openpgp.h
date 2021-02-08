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
#define OPENPGP_INS_GET_DATA                0xCA

#define TAG_AID                             0x4F
#define TAG_HISTORICAL_BYTES                0x5F52
#define TAG_PW_STATUS                       0xC4

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
