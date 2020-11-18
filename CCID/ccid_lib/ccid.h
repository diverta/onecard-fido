/* 
 * File:   ccid.h
 * Author: makmorit
 *
 * Created on 2020/05/29, 12:37
 */
#ifndef CCID_H
#define CCID_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// CCID関連定義
//
#define APDU_BUFFER_SIZE            1280
#define APDU_DATA_SIZE              (APDU_BUFFER_SIZE + 2)
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

//
// マクロ
//
#define LO(x) ((uint8_t)((x)&0x00FF))
#define HI(x) ((uint8_t)(((x)&0xFF00) >> 8))

//
// 関数群
//
void      ccid_initialize_value(void);
bool      ccid_data_frame_received(uint8_t *data, size_t len);
void      ccid_request_apdu_received(void);
uint8_t  *ccid_command_apdu_data(void);
size_t    ccid_command_apdu_size(void);
uint8_t  *ccid_response_apdu_data(void);
void      ccid_response_apdu_size_set(size_t size);
size_t    ccid_response_apdu_size_max(void);
void      ccid_resume_reader_to_pc_data_block(void);
void      ccid_response_time_extension(void);

#ifdef __cplusplus
}
#endif

#endif /* CCID_H */
