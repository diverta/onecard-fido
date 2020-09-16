/* 
 * File:   ccid_piv_object_import.h
 * Author: makmorit
 *
 * Created on 2020/09/16, 15:28
 */
#ifndef CCID_PIV_OBJECT_IMPORT_H
#define CCID_PIV_OBJECT_IMPORT_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint16_t ccid_piv_object_import(command_apdu_t *capdu, response_apdu_t *rapdu);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_OBJECT_IMPORT_H */
