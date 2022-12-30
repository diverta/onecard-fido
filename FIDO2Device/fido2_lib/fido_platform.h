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

// ラッパーモジュール群のヘッダーファイル
#include "ccid_crypto.h"
#include "ccid_flash_object.h"
#include "ccid_flash_oath_object.h"
#include "ccid_flash_openpgp_object.h"
#include "ccid_flash_piv_object.h"
#include "fido_ble_unpairing.h"
#include "fido_crypto.h"
#include "fido_flash.h"
#include "fido_timer.h"
#include "platform_common.h"
#include "rtcc.h"

#ifdef FIDO_ZEPHYR
// Zephyrに依存する処理
#include "app_platform.h"

#else
// nRF5 SDKに依存する処理
#include "fido_log.h"

#endif

#ifdef __cplusplus
}
#endif

#endif /* FIDO_PLATFORM_H */
