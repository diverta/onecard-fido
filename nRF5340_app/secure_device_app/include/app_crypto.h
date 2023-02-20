/* 
 * File:   app_crypto.h
 * Author: makmorit
 *
 * Created on 2021/05/12, 9:59
 */
#ifndef APP_CRYPTO_H
#define APP_CRYPTO_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void       *app_crypto_ctr_drbg_context(void);
void        app_crypto_do_process(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_CRYPTO_H */
