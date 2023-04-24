/* 
 * File:   app_bluetooth.h
 * Author: makmorit
 *
 * Created on 2021/04/06, 14:50
 */
#ifndef APP_BLUETOOTH_H
#define APP_BLUETOOTH_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        app_bluetooth_start(void);
void       *app_bluetooth_secure_connected_addr(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_BLUETOOTH_H */
