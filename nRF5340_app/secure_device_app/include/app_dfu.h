/* 
 * File:   app_dfu.h
 * Author: makmorit
 *
 * Created on 2021/08/11, 15:42
 */
#ifndef APP_DFU_H
#define APP_DFU_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        app_dfu_prepare_for_bootloader(void);
bool        app_dfu_commit(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_DFU_H */
