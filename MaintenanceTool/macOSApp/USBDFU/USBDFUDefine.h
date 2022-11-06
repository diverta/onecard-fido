//
//  USBDFUDefine.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/19.
//
#ifndef USBDFUDefine_h
#define USBDFUDefine_h

// 更新対象アプリケーション＝version 0.3.0
#define DFU_UPD_TARGET_APP_VERSION      300

// 処理タイムアウト（転送／反映チェック処理）
#define TIMEOUT_SEC_DFU_PROCESS         35.0

// イメージ反映所要時間（秒）
#define USBDFU_WAITING_SEC_ESTIMATED    15

// CDC ACM接続処理用の試行回数・インターバル
#define MAX_CNT_FOR_ACM_CONNECT         5
#define INTERVAL_SEC_FOR_ACM_CONNECT    1.0

// DFUコマンド応答タイムアウト
#define TIMEOUT_SEC_DFU_PING_RESPONSE   1.0
#define TIMEOUT_SEC_DFU_OPER_RESPONSE   3.0

// DFU関連の定義
#define NRF_DFU_BYTE_EOM                0xc0
#define NRF_DFU_BYTE_RESP_START         0x60
#define NRF_DFU_BYTE_RESP_SUCCESS       0x01

// DFUオブジェクト種別
#define NRF_DFU_BYTE_OBJ_INIT_CMD       0x01
#define NRF_DFU_BYTE_OBJ_DATA           0x02

// DFUコマンド種別（nRF52用）
#define NRF_DFU_OP_OBJECT_CREATE        0x01
#define NRF_DFU_OP_RECEIPT_NOTIF_SET    0x02
#define NRF_DFU_OP_CRC_GET              0x03
#define NRF_DFU_OP_OBJECT_EXECUTE       0x04
#define NRF_DFU_OP_OBJECT_SELECT        0x06
#define NRF_DFU_OP_MTU_GET              0x07
#define NRF_DFU_OP_OBJECT_WRITE         0x08
#define NRF_DFU_OP_PING                 0x09
#define NRF_DFU_OP_FIRMWARE_VERSION     0x0b

#endif /* USBDFUDefine_h */
