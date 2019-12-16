/* 
 * File:   fido_cryptoauth_setup.h
 * Author: makmorit
 *
 * Created on 2019/12/16, 15:18
 */
#ifndef FIDO_CRYPTOAUTH_SETUP_H
#define FIDO_CRYPTOAUTH_SETUP_H

#ifdef __cplusplus
extern "C" {
#endif

void fido_cryptoauth_setup_config_change(void);
void fido_cryptoauth_setup_config_lock(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_CRYPTOAUTH_SETUP_H */
