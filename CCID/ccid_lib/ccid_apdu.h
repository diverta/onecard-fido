/* 
 * File:   ccid_apdu.h
 * Author: makmorit
 *
 * Created on 2020/05/29, 15:21
 */
#ifndef CCID_APDU_H
#define CCID_APDU_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void ccid_apdu_process(void);
void ccid_apdu_stop_applet(void);

#ifdef __cplusplus
}
#endif

#endif /* CCID_APDU_H */
