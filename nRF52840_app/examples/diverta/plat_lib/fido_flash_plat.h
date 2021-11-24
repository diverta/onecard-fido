/* 
 * File:   fido_flash_plat.h
 * Author: makmorit
 *
 * Created on 2021/04/29, 10:15
 */
#ifndef FIDO_FLASH_PLAT_H
#define FIDO_FLASH_PLAT_H

#ifdef __cplusplus
extern "C" {
#endif

// Flash ROMに保存するための
// ファイルID、レコードKey
//
// File IDs should be in the range 0x0000 - 0xBFFF.
// Record keys should be in the range 0x0001 - 0xBFFF.
// https://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v15.0.0/lib_fds_functionality.html
//
#include "fido_flash_define.h"

//
//  ペアリングモード管理用
//
#define FIDO_PAIRING_MODE_FILE_ID     (0xBFFD)
#define FIDO_PAIRING_MODE_RECORD_KEY  (0xBFED)
//
// BLEペリフェラルによる自動認証パラメーター管理用
//   レコードサイズ = 11 ワード
//     スキャン対象サービスUUID文字列: 9ワード（36バイト)
//     スキャン秒数: 1ワード（4バイト）
//     自動認証フラグ: 1ワード（4バイト）
#define FIDO_BLP_AUTH_PARAM_FILE_ID         (0xBFF9)
#define FIDO_BLP_AUTH_PARAM_RECORD_KEY      (0xBFE9)
#define FIDO_BLP_AUTH_PARAM_RECORD_SIZE     11

//
// fido_flash_event.c
//
void fido_flash_event_set_gc_forced(void);
void fido_flash_fds_event_register(void);
void fido_flash_storage_init(void);

#ifdef __cplusplus
}
#endif

#endif // FIDO_FLASH_PLAT_H

/** @} */
