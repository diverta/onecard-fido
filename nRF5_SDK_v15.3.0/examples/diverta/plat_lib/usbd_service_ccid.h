/* 
 * File:   usbd_service_ccid.h
 * Author: makmorit
 *
 * Created on 2020/04/27, 11:04
 */
#ifndef USBD_SERVICE_CCID_H
#define USBD_SERVICE_CCID_H

#ifdef __cplusplus
extern "C" {
#endif

//
// CCID関連定義
//
#define CCID_CMD_HEADER_SIZE 10

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

//
// 関数群
//
void usbd_ccid_init(void);
void usbd_ccid_send_data_frame(uint8_t *p_data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* USBD_SERVICE_CCID_H */
