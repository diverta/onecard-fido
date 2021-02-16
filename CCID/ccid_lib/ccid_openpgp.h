/* 
 * File:   ccid_openpgp.h
 * Author: makmorit
 *
 * Created on 2021/02/08, 15:43
 */
#ifndef CCID_OPENPGP_H
#define CCID_OPENPGP_H

#include "ccid_apdu.h"

#ifdef __cplusplus
extern "C" {
#endif

// FOR TEST
#define OPENPGP_TEST_DATA

#define OPENPGP_INS_VERIFY                  0x20
#define OPENPGP_INS_SELECT                  0xA4
#define OPENPGP_INS_GET_DATA                0xCA

//
// データオブジェクトのタグ
//   以下はOpenPGPの仕様に存在しない定義
#define TAG_OPGP_NONE                       0x00
#define TAG_OPGP_PW1                        0x01
#define TAG_OPGP_PW3                        0x02
//   以下はOpenPGPの仕様に存在する定義
#define TAG_AID                             0x4F
#define TAG_NAME                            0x5B
#define TAG_LOGIN                           0x5E
#define TAG_LANG                            0x5F2D
#define TAG_SEX                             0x5F35
#define TAG_URL                             0x5F50
#define TAG_HISTORICAL_BYTES                0x5F52
#define TAG_CARDHOLDER_RELATED_DATA         0x65
#define TAG_APPLICATION_RELATED_DATA        0x6E
#define TAG_DISCRETIONARY_DATA_OBJECTS      0x73
#define TAG_SECURITY_SUPPORT_TEMPLATE       0x7A
#define TAG_EXTENDED_LENGTH_INFO            0x7F66
#define TAG_DIGITAL_SIG_COUNTER             0x93
#define TAG_EXTENDED_CAPABILITIES           0xC0
#define TAG_ALGORITHM_ATTRIBUTES_SIG        0xC1
#define TAG_ALGORITHM_ATTRIBUTES_DEC        0xC2
#define TAG_ALGORITHM_ATTRIBUTES_AUT        0xC3
#define TAG_PW_STATUS                       0xC4
#define TAG_KEY_FINGERPRINTS                0xC5
#define TAG_CA_FINGERPRINTS                 0xC6
#define TAG_KEY_SIG_FINGERPRINT             0xC7
#define TAG_KEY_DEC_FINGERPRINT             0xC8
#define TAG_KEY_AUT_FINGERPRINT             0xC9
#define TAG_KEY_CA1_FINGERPRINT             0xCA
#define TAG_KEY_CA2_FINGERPRINT             0xCB
#define TAG_KEY_CA3_FINGERPRINT             0xCC
#define TAG_KEY_GENERATION_DATES            0xCD
#define TAG_KEY_SIG_GENERATION_DATES        0xCE
#define TAG_KEY_DEC_GENERATION_DATES        0xCF
#define TAG_KEY_AUT_GENERATION_DATES        0xD0
#define TAG_KEY_INFO                        0xDE

#define OPGP_MAX_CERT_LENGTH                0x480
#define OPGP_MAX_DO_LENGTH                  0xFF

//
// 関数群
//
bool ccid_openpgp_aid_is_applet(command_apdu_t *capdu);
void ccid_openpgp_apdu_process(command_apdu_t *capdu, response_apdu_t *rapdu);
void ccid_openpgp_stop_applet(void);

#ifdef __cplusplus
}
#endif

#endif /* CCID_OPENPGP_H */
