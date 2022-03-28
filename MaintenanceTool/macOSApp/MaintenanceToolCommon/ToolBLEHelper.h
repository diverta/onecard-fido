//
//  ToolBLEHelper.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/07.
//
#ifndef ToolBLEHelper_h
#define ToolBLEHelper_h

// エラー種別
typedef enum : NSInteger {
    BLE_ERR_BLUETOOTH_OFF,
    BLE_ERR_DEVICE_CONNECT_FAILED,
    BLE_ERR_DEVICE_CONNREQ_TIMEOUT,
    BLE_ERR_DEVICE_SCAN_TIMEOUT,
    BLE_ERR_SERVICE_NOT_DISCOVERED,
    BLE_ERR_SERVICE_NOT_FOUND,
    BLE_ERR_DISCOVER_SERVICE_TIMEOUT,
    BLE_ERR_CHARACT_NOT_DISCOVERED,
    BLE_ERR_DISCOVER_CHARACT_TIMEOUT,
    BLE_ERR_CHARACT_NOT_EXIST,
    BLE_ERR_NOTIFICATION_FAILED,
    BLE_ERR_NOTIFICATION_STOP,
    BLE_ERR_SUBSCRIBE_CHARACT_TIMEOUT,
    BLE_ERR_REQUEST_SEND_FAILED,
    BLE_ERR_RESPONSE_RECEIVE_FAILED,
    BLE_ERR_REQUEST_TIMEOUT,
} BLEErrorReason;

@protocol ToolBLEHelperDelegate;

@interface ToolBLEHelper : NSObject

    - (id)initWithDelegate:(id<ToolBLEHelperDelegate>)delegate;
    - (void)helperWillConnectWithUUID:(NSString *)uuidString;
    - (void)helperWillDisconnect;
    - (void)helperWillConnectPeripheral:(id)peripheralRef;
    - (void)helperWillDiscoverServiceWithUUID:(NSString *)uuidString;
    - (void)helperWillDiscoverCharacteristicsWithUUIDs:(NSArray<NSString *> *)uuids;
    - (void)helperWillSubscribeCharacteristicWithTimeout:(NSTimeInterval)timeoutSec;
    - (void)helperWillWriteForCharacteristics:(NSData *)requestMessage;
    - (void)helperWillReadForCharacteristics;

@end

@protocol ToolBLEHelperDelegate <NSObject>

    - (void)helperDidScanForPeripheral:(id)peripheralRef withUUID:(NSString *)uuidString;
    - (void)helperDidConnectPeripheral;
    - (void)helperDidFailConnectionWithError:(NSError *)error reason:(BLEErrorReason)reason;
    - (void)helperDidDisconnectWithError:(NSError *)error;
    - (void)helperDidDiscoverService;
    - (void)helperDidDiscoverCharacteristics;
    - (void)helperDidSubscribeCharacteristic;
    - (void)helperDidWriteForCharacteristics;
    - (void)helperDidReadForCharacteristic:(NSData *)responseMessage;

@end

#endif /* ToolBLEHelper_h */
