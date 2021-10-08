//
//  ToolBLEHelper.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/07.
//
#ifndef ToolBLEHelper_h
#define ToolBLEHelper_h

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

// エラー種別
typedef enum : NSInteger {
    BLE_ERR_BLUETOOTH_OFF,
    BLE_ERR_DEVICE_CONNECT_FAILED,
    BLE_ERR_DEVICE_CONNREQ_TIMEOUT,
    BLE_ERR_DEVICE_DISCONNECTED,
    BLE_ERR_DEVICE_SCAN_START,
    BLE_ERR_DEVICE_SCAN_STOPPED,
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
    - (void)helperWillDiscoverServiceWithUUID:(NSString *)uuidString;
    - (void)helperWillDiscoverCharacteristicsWithUUIDs:(NSArray<NSString *> *)uuids;
    - (void)helperWillWriteForCharacteristics:(NSData *)requestMessage;
    - (void)helperWillReadForCharacteristics;

@end

@protocol ToolBLEHelperDelegate <NSObject>

    - (void)notifyCentralManagerStateUpdate:(CBCentralManagerState)state;
    - (void)helperDidConnectPeripheral;
    - (void)helperDidFailConnectionWith:(BLEErrorReason)reason error:(NSError *)error;
    - (void)helperDidDisconnect;
    - (void)helperDidDiscoverService;
    - (void)helperDidDiscoverCharacteristics;
    - (void)helperDidWriteForCharacteristics;
    - (void)helperDidReadForCharacteristic:(NSData *)responseMessage;
    - (void)helperNotifyStatus:(BLEErrorReason)reason error:(NSError *)error;

@end

#endif /* ToolBLEHelper_h */
