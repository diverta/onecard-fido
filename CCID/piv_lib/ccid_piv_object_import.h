/* 
 * File:   ccid_piv_object_import.h
 * Author: makmorit
 *
 * Created on 2020/09/16, 15:28
 */
#ifndef CCID_PIV_OBJECT_IMPORT_H
#define CCID_PIV_OBJECT_IMPORT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint16_t ccid_piv_object_import(void *p_capdu, void *p_rapdu);
void     ccid_piv_object_import_retry(void);
void     ccid_piv_object_import_resume(bool success);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_OBJECT_IMPORT_H */
