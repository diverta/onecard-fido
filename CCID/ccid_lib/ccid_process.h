/* 
 * File:   ccid_process.h
 * Author: makmorit
 *
 * Created on 2022/04/29, 9:07
 */
#ifndef CCID_PROCESS_H
#define CCID_PROCESS_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        ccid_process_stop_applet(void);
void        ccid_process_applet(void *p_capdu, void *p_rapdu);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PROCESS_H */
