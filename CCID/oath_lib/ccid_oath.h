/* 
 * File:   ccid_oath.h
 * Author: makmorit
 *
 * Created on 2022/04/29, 10:42
 */
#ifndef CCID_OATH_H
#define CCID_OATH_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        ccid_oath_aid_is_applet(void *p_capdu);
void        ccid_oath_apdu_process(void *p_capdu, void *p_rapdu);
void        ccid_oath_stop_applet(void);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OATH_H */
