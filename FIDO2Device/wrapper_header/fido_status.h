/* 
 * File:   fido_status.h
 * Author: makmorit
 *
 * Created on 2023/02/07, 12:34
 */
#ifndef FIDO_STATUS_H
#define FIDO_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        fido_status_set_to_idle(void);
void        fido_status_set_to_busy(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_STATUS_H */
