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

#define NFC_APDU_BUFF_SIZE 256

void nfc_func_init(void);
void nfc_func_main(void);

#ifdef __cplusplus
}
#endif

#endif /* NFC_SERVICE_H */

