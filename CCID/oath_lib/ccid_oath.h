/* 
 * File:   ccid_oath.h
 * Author: makmorit
 *
 * Created on 2022/04/29, 10:42
 */
#ifndef CCID_OATH_H
#define CCID_OATH_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        ccid_oath_aid_is_applet(command_apdu_t *capdu);
void        ccid_oath_apdu_process(command_apdu_t *capdu, response_apdu_t *rapdu);
void        ccid_oath_stop_applet(void);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OATH_H */
