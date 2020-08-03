/* 
 * File:   ccid_piv_object.h
 * Author: makmorit
 *
 * Created on 2020/06/02, 11:06
 */
#ifndef CCID_PIV_OBJECT_H
#define CCID_PIV_OBJECT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// データオブジェクトのタグ
//
#define TAG_KEY_PAUTH   0x9a
#define TAG_KEY_CAADM   0x9b
#define TAG_KEY_DGSIG   0x9c
#define TAG_KEY_KEYMN   0x9d
#define TAG_KEY_CAUTH   0x9e

//
// データオブジェクト関連定義
//
#define CAADM_KEY_SIZE  24

//
// 関数群
//
bool    ccid_piv_object_chuid_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_ccc_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_cert_cauth_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_key_pauth_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_cert_pauth_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_key_digsig_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_cert_digsig_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_key_keyman_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_cert_keyman_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_key_history_get(uint8_t *buffer, size_t *size);
bool    ccid_piv_object_card_admin_key_get(uint8_t *buffer, size_t *size);
uint8_t ccid_piv_object_card_admin_key_alg_get(void);
bool    ccid_piv_object_get(uint8_t data_obj_tag, uint8_t *buffer, size_t *size);

#ifdef __cplusplus
}
#endif

#endif /* CCID_PIV_OBJECT_H */
