/* 
 * File:   fido_common.h
 * Author: makmorit
 *
 * Created on 2018/12/18, 11:09
 */
#ifndef FIDO_COMMON_H
#define FIDO_COMMON_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// リクエストデータに含まれるAPDU項目を保持
typedef struct {
    uint8_t  CLA;
    uint8_t  INS;
    uint8_t  P1;
    uint8_t  P2;
    uint32_t Lc;
    uint8_t *data;
    uint32_t data_length;
    uint32_t Le;
} FIDO_APDU_T;

// APDUに格納できるデータ長の上限
#ifndef APDU_DATA_MAX_LENGTH
#define APDU_DATA_MAX_LENGTH 1024
#endif

// トランスポート種別
typedef enum _TRANSPORT_TYPE {
    TRANSPORT_NONE = 0,
    TRANSPORT_BLE,
    TRANSPORT_HID,
    TRANSPORT_NFC
} TRANSPORT_TYPE;

//
// INITコマンドのレスポンスデータ編集領域
//   固定長（17バイト）
//   U2FHID_INIT、CTAPHID_INITで利用
//
typedef struct {
    uint8_t nonce[8];
    uint8_t cid[4];
    uint8_t version_id;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_build;
    uint8_t cflags;
} HID_INIT_RES_T;

// 関数群
void     fido_set_status_word(uint8_t *dest_buffer, uint16_t status_word);
void     fido_set_uint32_bytes(uint8_t *p_dest_buffer, uint32_t bytes);
void     fido_set_uint16_bytes(uint8_t *p_dest_buffer, uint16_t bytes);
uint16_t fido_get_uint16_from_bytes(uint8_t *p_src_buffer);
uint32_t fido_get_uint32_from_bytes(uint8_t *p_src_buffer);
uint64_t fido_get_uint64_from_bytes(uint8_t *p_src_buffer);
size_t   fido_calculate_aes_block_size(size_t buffer_size);
uint8_t *fido_extract_pubkey_in_certificate(uint8_t *cert_data, size_t cert_data_length);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_COMMON_H */
