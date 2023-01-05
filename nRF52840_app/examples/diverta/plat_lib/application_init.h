/* 
 * File:   application_init.h
 * Author: makmorit
 *
 * Created on 2020/08/17, 10:14
 */
#ifndef APPLICATION_INIT_H
#define APPLICATION_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void         application_init_start(void);
void         application_init_ble_pairing_has_reset(void);
void         application_main(void);

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_INIT_H */
