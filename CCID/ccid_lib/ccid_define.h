/* 
 * File:   ccid_define.h
 * Author: makmorit
 *
 * Created on 2023/01/02, 12:05
 */
#ifndef CCID_DEFINE_H
#define CCID_DEFINE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// CCID関連定義
//
#define CCID_CMD_HEADER_SIZE        10
#define CCID_NUMBER_OF_SLOTS        1

// コマンド (PC-->Reader)
#define PC_TO_RDR_ICCPOWERON        0x62
#define PC_TO_RDR_ICCPOWEROFF       0x63
#define PC_TO_RDR_GETSLOTSTATUS     0x65
#define PC_TO_RDR_XFRBLOCK          0x6F
#define PC_TO_RDR_GETPARAMETERS     0x6C
#define PC_TO_RDR_RESETPARAMETERS   0x6D
#define PC_TO_RDR_SETPARAMETERS     0x61
#define PC_TO_RDR_ESCAPE            0x6B
#define PC_TO_RDR_ICCCLOCK          0x6E
#define PC_TO_RDR_T0APDU            0x6A
#define PC_TO_RDR_SECURE            0x69
#define PC_TO_RDR_MECHANICAL        0x71
#define PC_TO_RDR_ABORT             0x72
#define PC_TO_RDR_SETDATARATEANDCLOCKFREQUENCY 0x73

// コマンド (Reader-->PC)
#define RDR_TO_PC_DATABLOCK         0x80
#define RDR_TO_PC_SLOTSTATUS        0x81
#define RDR_TO_PC_PARAMETERS        0x82
#define RDR_TO_PC_ESCAPE            0x83
#define RDR_TO_PC_DATARATEANDCLOCKFREQUENCY 0x84

// エラーステータス
#define SLOT_NO_ERROR               0x81
#define SLOTERROR_CMD_NOT_SUPPORTED 0x00
#define SLOTERROR_BAD_POWERSELECT   0x07

// 各種ステータス
#define BM_ICC_PRESENT_ACTIVE       0x00
#define BM_ICC_PRESENT_INACTIVE     0x01

#define BM_COMMAND_STATUS_NO_ERROR  0x00
#define BM_COMMAND_STATUS_OFFSET    0x06
#define BM_COMMAND_STATUS_FAILED    (0x01 << BM_COMMAND_STATUS_OFFSET)
#define BM_COMMAND_STATUS_TIME_EXTN (0x02 << BM_COMMAND_STATUS_OFFSET)

// コマンド
#define INS_GET_RESPONSE_APDU       0xC0

//
// マクロ
//
#define LO(x) ((uint8_t)((x)&0x00FF))
#define HI(x) ((uint8_t)(((x)&0xFF00) >> 8))

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

typedef enum {
    APPLET_NONE,
    APPLET_PIV,
    APPLET_OATH,
    APPLET_OPENPGP
} CCID_APPLET;

#ifdef __cplusplus
}
#endif

#endif /* CCID_DEFINE_H */
