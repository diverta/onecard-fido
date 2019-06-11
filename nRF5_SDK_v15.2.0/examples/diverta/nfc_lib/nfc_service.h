/* 
 * File:   nfc_service.h
 * Author: makmorit
 *
 * Created on 2019/05/28, 14:21
 */
#ifndef NFC_SERVICE_H
#define NFC_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"

//
// 関数群
//
void nfc_service_init(bool closure);
void nfc_service_data_send(uint8_t *data, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif /* NFC_SERVICE_H */

