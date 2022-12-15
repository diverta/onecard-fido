//
//  ToolBLEHelperDefine.h
//  ToolCommon
//
//  Created by Makoto Morita on 2022/07/12.
//
#ifndef ToolBLEHelperDefine_h
#define ToolBLEHelperDefine_h

#import <Foundation/Foundation.h>

// エラー種別
typedef enum : NSUInteger {
    BLE_ERR_BLUETOOTH_OFF,
    BLE_ERR_DEVICE_CONNECT_FAILED,
    BLE_ERR_DEVICE_CONNREQ_TIMEOUT,
    BLE_ERR_DEVICE_SCAN_TIMEOUT,
    BLE_ERR_SERVICE_NOT_DISCOVERED,
    BLE_ERR_SERVICE_NOT_FOUND,
    BLE_ERR_CHARACT_NOT_DISCOVERED,
    BLE_ERR_CHARACT_NOT_EXIST,
    BLE_ERR_NOTIFICATION_FAILED,
    BLE_ERR_NOTIFICATION_STOP,
    BLE_ERR_REQUEST_SEND_FAILED,
    BLE_ERR_RESPONSE_RECEIVE_FAILED,
    BLE_ERR_REQUEST_TIMEOUT,
} BLEErrorReason;

#endif /* ToolBLEHelperDefine_h */
