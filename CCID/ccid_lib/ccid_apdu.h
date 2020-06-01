/* 
 * File:   ccid_apdu.h
 * Author: makmorit
 *
 * Created on 2020/05/29, 15:21
 */
#ifndef CCID_APDU_H
#define CCID_APDU_H

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
// 関数群
//
void ccid_apdu_process(void);
void ccid_apdu_stop_applet(void);

#ifdef __cplusplus
}
#endif

#endif /* CCID_APDU_H */
