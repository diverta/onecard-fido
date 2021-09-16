/* 
 * File:   app_fido.h
 * Author: makmorit
 *
 * Created on 2021/09/16, 10:03
 */
#ifndef APP_FIDO_H
#define APP_FIDO_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint8_t    *fido_crypto_sskey_public_key(void);
void        fido_crypto_sskey_init(bool force);

#ifdef __cplusplus
}
#endif

#endif /* APP_FIDO_H */
