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

// Capability Container
typedef struct {
    uint8_t cclen_hi;
    uint8_t cclen_lo;
    uint8_t version;
    uint8_t MLe_hi;
    uint8_t MLe_lo;
    uint8_t MLc_hi;
    uint8_t MLc_lo;
    uint8_t tlv[8];
} __attribute__((packed)) CAPABILITY_CONTAINER;

// APDUヘッダー（5バイト）    
typedef struct {
    uint8_t cla;
    uint8_t ins;
    uint8_t p1;
    uint8_t p2;
    uint8_t lc;
} __attribute__((packed)) APDU_HEADER;
#define APDU_HEADER_SIZE 5

// APDUコマンドコード
#define APDU_FIDO_U2F_REGISTER        0x01
#define APDU_FIDO_U2F_AUTHENTICATE    0x02
#define APDU_FIDO_U2F_VERSION         0x03
#define APDU_FIDO_NFCCTAP_MSG         0x10
#define APDU_INS_SELECT               0xA4
#define APDU_INS_READ_BINARY          0xB0

// APDUレスポンスコード
#define SW_SUCCESS                    0x9000
#define SW_GET_RESPONSE               0x6100
#define SW_WRONG_LENGTH               0x6700
#define SW_COND_USE_NOT_SATISFIED     0x6985
#define SW_FILE_NOT_FOUND             0x6a82
#define SW_INS_INVALID                0x6d00
#define SW_INTERNAL_EXCEPTION         0x6f00

//
// 関数群
//
void nfc_service_init(void);
void nfc_service_data_send(uint8_t *data, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif /* NFC_SERVICE_H */

