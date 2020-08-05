/* 
 * File:   ccid_apdu.h
 * Author: makmorit
 *
 * Created on 2020/05/29, 15:21
 */
#ifndef CCID_APDU_H
#define CCID_APDU_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "ccid.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// Command status responses
//
#define SW_NO_ERROR                 0x9000
#define SW_PIN_RETRIES              0x63C0
#define SW_WRONG_LENGTH             0x6700
#define SW_UNABLE_TO_PROCESS        0x6900
#define SW_SECURITY_STATUS_NOT_SATISFIED 0x6982
#define SW_AUTHENTICATION_BLOCKED   0x6983
#define SW_DATA_INVALID             0x6984
#define SW_CONDITIONS_NOT_SATISFIED 0x6985
#define SW_COMMAND_NOT_ALLOWED      0x6986
#define SW_WRONG_DATA               0x6A80
#define SW_FILE_NOT_FOUND           0x6A82
#define SW_NOT_ENOUGH_SPACE         0x6A84
#define SW_WRONG_P1P2               0x6A86
#define SW_REFERENCE_DATA_NOT_FOUND 0x6A88
#define SW_INS_NOT_SUPPORTED        0x6D00
#define SW_CLA_NOT_SUPPORTED        0x6E00
#define SW_CHECKING_ERROR           0x6F00

//
// 構造体定義
//
typedef struct command_apdu {
    uint8_t  cla;
    uint8_t  ins;
    uint8_t  p1;
    uint8_t  p2;
    size_t   lc;
    size_t   le;
    uint8_t *data;
} command_apdu_t;

typedef struct response_apdu {
    uint8_t  data[APDU_BUFFER_SIZE];
    uint16_t len;
    uint16_t sw;
    size_t   already_sent;
} response_apdu_t;

//
// 関数群
//
void ccid_apdu_process(void);
void ccid_apdu_resume_process(command_apdu_t *capdu, response_apdu_t *rapdu);
void ccid_apdu_stop_applet(void);
bool ccid_apdu_response_is_pending(void);
void ccid_apdu_response_set_pending(bool b);

#ifdef __cplusplus
}
#endif

#endif /* CCID_APDU_H */
