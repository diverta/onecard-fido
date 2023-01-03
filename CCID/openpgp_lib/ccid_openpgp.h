/* 
 * File:   ccid_openpgp.h
 * Author: makmorit
 *
 * Created on 2021/02/08, 15:43
 */
#ifndef CCID_OPENPGP_H
#define CCID_OPENPGP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool ccid_openpgp_aid_is_applet(void *p_capdu);
void ccid_openpgp_apdu_process(void *p_capdu, void *p_rapdu);
void ccid_openpgp_stop_applet(void);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_H */
