/* 
 * File:   fido_platform.h
 * Author: makmorit
 *
 * Created on 2019/06/24, 10:45
 */
#ifndef FIDO_PLATFORM_H
#define FIDO_PLATFORM_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// crypto関連の共通定義
#define RAW_PRIVATE_KEY_SIZE    32
#define RAW_PUBLIC_KEY_SIZE     64
#define SHARED_SECRET_SIZE      32
#define ECDSA_SIGNATURE_SIZE    64
#define SHA_256_HASH_SIZE       32
#define SSKEY_HASH_SIZE         32
#define HMAC_SHA_256_SIZE       32

#ifdef FIDO_ZEPHYR
// Zephyrに依存しない処理
#include "ccid_crypto.h"
#include "ccid_flash_object.h"
#include "ccid_flash_openpgp_object.h"
#include "ccid_flash_piv_object.h"
#include "fido_crypto.h"
#include "fido_flash.h"
#include "fido_timer.h"

// Zephyrに依存する処理
#include "app_crypto.h"
#include "app_crypto_ec.h"
#include "app_platform.h"
#include "app_rtcc.h"
#include "app_settings.h"
#include "app_timer.h"

#else
// ハードウェアの差異に依存しない定義を集約
#include "fido_platform_common.h"

// ハードウェアの差異に依存する定義を集約
#include "ble_service_common.h"
#include "ccid_crypto.h"
#include "fido_crypto_plat.h"
#include "fido_crypto_keypair.h"
#include "fido_crypto_sskey.h"

//
// fido_flash_skey_cert.c
//
bool      fido_flash_skey_cert_delete(void);
bool      fido_flash_skey_cert_write(void);
bool      fido_flash_skey_cert_read(void);
bool      fido_flash_skey_cert_available(void);
bool      fido_flash_skey_cert_data_prepare(uint8_t *data, uint16_t length);
uint32_t *fido_flash_skey_cert_data(void);
uint8_t  *fido_flash_skey_data(void);
uint8_t  *fido_flash_cert_data(void);
uint32_t  fido_flash_cert_data_length(void);

//
// fido_flash_password.c
//
uint8_t *fido_flash_password_get(void);
bool     fido_flash_password_set(uint8_t *random_vector);
#endif

#ifdef __cplusplus
}
#endif

#endif /* FIDO_PLATFORM_H */
