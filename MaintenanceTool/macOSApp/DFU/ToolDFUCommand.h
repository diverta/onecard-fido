//
//  ToolDFUCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/12/31.
//
#ifndef ToolDFU_h
#define ToolDFU_h

// DFU関連の定義
#define NRF_DFU_BYTE_EOM             0xc0
#define NRF_DFU_BYTE_RESP_START      0x60
#define NRF_DFU_BYTE_RESP_SUCCESS    0x01

// DFUオブジェクト種別
#define NRF_DFU_BYTE_OBJ_INIT_CMD    0x01
#define NRF_DFU_BYTE_OBJ_DATA        0x02

// DFUコマンド種別（nRF52用）
#define NRF_DFU_OP_OBJECT_CREATE     0x01
#define NRF_DFU_OP_RECEIPT_NOTIF_SET 0x02
#define NRF_DFU_OP_CRC_GET           0x03
#define NRF_DFU_OP_OBJECT_EXECUTE    0x04
#define NRF_DFU_OP_OBJECT_SELECT     0x06
#define NRF_DFU_OP_MTU_GET           0x07
#define NRF_DFU_OP_OBJECT_WRITE      0x08
#define NRF_DFU_OP_PING              0x09

@interface ToolDFUCommand : NSObject

    - (NSString *)testMain;

@end

#endif /* ToolDFU_h */
