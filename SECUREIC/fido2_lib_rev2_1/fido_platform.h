/* 
 * File:   fido_platform.h
 * Author: makmorit
 *
 * Created on 2019/06/24, 10:45
 */
#ifndef FIDO_PLATFORM_H
#define FIDO_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

// ハードウェアの差異に依存しない定義を集約
#include "fido_platform_common.h"

// ハードウェアの差異に依存する定義を集約
#include "fido_crypto.h"
#include "fido_crypto_keypair.h"
#include "fido_crypto_sskey.h"
#include "fido_flash_skey_cert.h"
    
#ifdef __cplusplus
}
#endif

#endif /* FIDO_PLATFORM_H */
