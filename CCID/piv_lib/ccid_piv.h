/* 
 * File:   ccid_piv.h
 * Author: makmorit
 *
 * Created on 2020/06/01, 9:55
 */
#ifndef CCID_PIV_H
#define CCID_PIV_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool ccid_piv_rid_is_piv_applet(void *p_capdu);

void ccid_piv_apdu_process(void *p_capdu, void *p_rapdu);
void ccid_piv_stop_applet(void);
bool ccid_piv_admin_mode_get(void);
void ccid_piv_admin_mode_set(bool mode);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_H */
