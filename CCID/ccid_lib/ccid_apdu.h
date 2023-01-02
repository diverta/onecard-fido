/* 
 * File:   ccid_apdu.h
 * Author: makmorit
 *
 * Created on 2020/05/29, 15:21
 */
#ifndef CCID_APDU_H
#define CCID_APDU_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void ccid_apdu_process(void);
void ccid_apdu_resume_process(void *p_capdu, void *p_rapdu);
bool ccid_apdu_response_is_pending(void);
void ccid_apdu_response_set_pending(bool b);
void ccid_apdu_assert(void *p_capdu, void *p_rapdu);

#ifdef __cplusplus
}
#endif

#endif /* CCID_APDU_H */
