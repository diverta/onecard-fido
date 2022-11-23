/* 
 * File:   ccid_process.h
 * Author: makmorit
 *
 * Created on 2022/04/29, 9:07
 */
#ifndef CCID_PROCESS_H
#define CCID_PROCESS_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        ccid_process_stop_applet(void);
void        ccid_process_applet(command_apdu_t *capdu, response_apdu_t *rapdu);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PROCESS_H */
