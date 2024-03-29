//
//  ToolBLEHelper.m
//  ToolCommon
//
//  Created by Makoto Morita on 2022/07/12.
//
#import <CoreBluetooth/CoreBluetooth.h>

#import "AppCommonMessage.h"
#import "ToolBLEHelper.h"
#import "ToolBLEHelperDefine.h"
#import "ToolLogFile.h"

@interface ToolBLEHelper () <CBCentralManagerDelegate, CBPeripheralDelegate>

    @property(nonatomic, weak) id<ToolBLEHelperDelegate> delegate;
    @property(nonatomic) CBCentralManager   *manager;
    @property(nonatomic) CBPeripheral       *discoveredPeripheral;
    @property(nonatomic) CBCharacteristic   *characteristicForWrite;
    @property(nonatomic) CBCharacteristic   *characteristicForWriteNoResp;
    @property(nonatomic) CBCharacteristic   *characteristicForNotify;
    @property(nonatomic, strong) NSArray    *serviceUUIDs;

@end

@implementation ToolBLEHelper

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolBLEHelperDelegate>)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
            [self setManager:[[CBCentralManager alloc] initWithDelegate:self queue:nil]];
            [self clearDiscoveredPeripheral];
        }
        return self;
    }

    - (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    }

    - (void)clearDiscoveredPeripheral {
        // BLEペリフェラルの参照をクリア（参照はスキャン成功時に設定される）
        [self setDiscoveredPeripheral:nil];
    }

#pragma mark - Entry for process

    - (void)helperWillConnectWithUUID:(NSString *)uuidString {
        // すでに接続が確立されている場合は通知
        if ([self discoveredPeripheral] != nil) {
            [[ToolLogFile defaultLogger] error:@"helperWillConnectWithUUID: Connected peripheral already exist"];
            [[self delegate] helperDidFailConnectionWithError:nil reason:BLE_ERR_DEVICE_CONNECT_FAILED];
            return;
        }
        // BLEが無効化されている場合は通知
        if ([[self manager] state] != CBManagerStatePoweredOn) {
            [[self delegate] helperDidFailConnectionWithError:nil reason:BLE_ERR_BLUETOOTH_OFF];
            return;
        }
        // 引数のサービスUUIDを有するペリフェラルのスキャン開始
        [self setServiceUUIDs:@[[CBUUID UUIDWithString:uuidString]]];
        [self scanForPeripherals];
    }

#pragma mark - Scan for peripherals

    - (void)scanForPeripherals {
        // スキャン設定
        NSDictionary *scanningOptions = @{CBCentralManagerScanOptionAllowDuplicatesKey : @NO};
        // BLEペリフェラルをスキャン
        [self clearDiscoveredPeripheral];
        [[self manager] scanForPeripheralsWithServices:nil options:scanningOptions];
        [[ToolLogFile defaultLogger] info:MSG_U2F_DEVICE_SCAN_START];
        // スキャンタイムアウト監視を開始
        [self startScanningTimeoutMonitor];
    }

    - (void)cancelScanForPeripherals {
        // スキャンを停止
        [[self manager] stopScan];
        [[ToolLogFile defaultLogger] info:MSG_U2F_DEVICE_SCAN_STOPPED];
    }

    - (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral
         advertisementData:(NSDictionary *)advertisementData
                      RSSI:(NSNumber *)RSSI {
        // 所定のサービスUUIDを有するペリフェラルかどうか判定
        NSArray *serviceUUIDs = [advertisementData objectForKey:CBAdvertisementDataServiceUUIDsKey];
        for (CBUUID *foundServiceUUIDs in serviceUUIDs) {
            if ([[self serviceUUIDs] containsObject:foundServiceUUIDs] == false) {
                continue;
            }
            // サービスデータフィールドを取得
            NSData *serviceDataField = [self retrieveServiceDataFieldFrom:advertisementData withUUID:foundServiceUUIDs];
            // ペリフェラルの参照を保持（`API MISUSE: Cancelling connection for unused peripheral`というエラー発生の回避措置）
            [self setDiscoveredPeripheral:peripheral];
            // スキャンタイムアウト監視を停止
            [self cancelScanningTimeoutMonitor];
            // スキャンを停止し、スキャン完了を通知
            [self cancelScanForPeripherals];
            NSString *scannedPeripheralName = [advertisementData objectForKey:CBAdvertisementDataLocalNameKey];
            [[self delegate] helperDidScanForPeripheral:peripheral scannedPeripheralName:scannedPeripheralName
                                               withUUID:[foundServiceUUIDs UUIDString] withServiceData:serviceDataField];
            break;
        }
    }

    - (void)scanningDidTimeout {
        // スキャンを停止
        [self cancelScanForPeripherals];
        // スキャンタイムアウトの場合は通知
        [[self delegate] helperDidFailConnectionWithError:nil reason:BLE_ERR_DEVICE_SCAN_TIMEOUT];
    }

    - (NSData *)retrieveServiceDataFieldFrom:(NSDictionary *)advertisementData withUUID:(CBUUID *)uuid {
        // アドバタイズデータから、所定のサービスUUIDに対応するサービスデータフィールドを抽出
        NSData *serviceDataField = nil;
        NSDictionary *serviceData = [advertisementData objectForKey:CBAdvertisementDataServiceDataKey];
        if (serviceData != nil) {
            serviceDataField = [serviceData objectForKey:uuid];
        }
        return serviceDataField;
    }

#pragma mark - Connect peripheral

    - (void)helperWillConnectPeripheral:(id)peripheralRef {
        // ペリフェラルに接続
        CBPeripheral *peripheral = (CBPeripheral *)peripheralRef;
        [[self manager] connectPeripheral:peripheral options:nil];
    }

    - (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral {
        // 接続完了を通知
        [[self delegate] helperDidConnectPeripheral];
    }

    - (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral
                     error:(NSError *)error {
        // 接続失敗を通知
        [[self delegate] helperDidFailConnectionWithError:error reason:BLE_ERR_DEVICE_CONNECT_FAILED];
    }

    - (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral
                     error:(NSError *)error {
        // レスポンスタイムアウト監視を停止
        if ([self characteristicForNotify]) {
            [self cancelResponseTimeoutMonitor:[self characteristicForNotify]];
        }
        // 接続参照を解除
        [self setDiscoveredPeripheral:nil];
        // 切断が正常完了した場合
        if ([error code] == 0) {
            [[ToolLogFile defaultLogger] debug:@"BLE connection has terminated successfully."];
            [self clearDiscoveredPeripheral];
            [[self delegate] helperDidDisconnectWithError:nil peripheral:nil];
        }
        // ペリフェラル側からの一方的な切断による接続タイムアウトの場合＝切断済み
        else if ([[error domain] isEqualTo:CBErrorDomain] && [error code] == 6) {
            [[ToolLogFile defaultLogger] error:@"BLE connection has terminated unexpectedly."];
            [self clearDiscoveredPeripheral];
            [[self delegate] helperDidDisconnectWithError:error peripheral:nil];
        }
        // その他の場合は不明なエラーと扱い、内容詳細をログ出力
        else {
            [[ToolLogFile defaultLogger] errorWithFormat:@"BLE disconnected with message: %@", [error description]];
            [self clearDiscoveredPeripheral];
            [[self delegate] helperDidDisconnectWithError:error peripheral:nil];
        }
    }

#pragma mark - Discover services

    - (void)helperWillDiscoverServiceWithUUID:(NSString *)uuidString {
        // サービスのディスカバーを開始
        CBPeripheral *peripheral = [self discoveredPeripheral];
        [peripheral setDelegate:self];
        [peripheral discoverServices:nil];
        [self setServiceUUIDs:@[[CBUUID UUIDWithString:uuidString]]];
    }

    - (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error {
        // BLEサービスディスカバーに失敗の場合
        if (error) {
            // ディスカバー失敗を通知
            [[self delegate] helperDidFailConnectionWithError:error reason:BLE_ERR_SERVICE_NOT_DISCOVERED];
            return;
        }
        // サービスを判定し、その参照を保持
        CBService *connectedService = nil;
        for (CBService *service in [peripheral services]) {
            if ([[self serviceUUIDs] containsObject:service.UUID]) {
                connectedService = service;
                break;
            }
        }
        // サービスがない場合
        if (connectedService == nil) {
            // サービスがない旨を通知
            [[self delegate] helperDidFailConnectionWithError:nil reason:BLE_ERR_SERVICE_NOT_FOUND];
            return;
        }
        // ディスカバー完了を通知
        [[self delegate] helperDidDiscoverService:connectedService];
    }

#pragma mark - Discover characteristics

    - (void)helperWillDiscoverCharacteristics:(id)serviceRef withUUIDs:(NSArray<NSString *> *)uuids {
        // ディスカバー対象のキャラクタリスティックUUIDを保持
        NSMutableArray *array = [[NSMutableArray alloc] init];
        for (NSString *uuidString in uuids) {
            [array addObject:[CBUUID UUIDWithString:uuidString]];
        }
        // サービス内のキャラクタリスティックをディスカバー
        [[self discoveredPeripheral] discoverCharacteristics:array forService:(CBService *)serviceRef];
    }

    - (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service
            error:(NSError *)error {
        // キャラクタリスティックのディスカバーに失敗の場合
        if (error) {
            // ディスカバー失敗を通知
            [[self delegate] helperDidFailConnectionWithError:error reason:BLE_ERR_CHARACT_NOT_DISCOVERED];
            return;
        }
        // サービスにキャラクタリスティックがない場合は通知
        if ([[service characteristics] count] < 1) {
            // キャラクタリスティックがない旨を通知
            [[self delegate] helperDidFailConnectionWithError:nil reason:BLE_ERR_CHARACT_NOT_EXIST];
            return;
        }
        // ディスカバー完了を通知
        [[self delegate] helperDidDiscoverCharacteristics:service];
    }

#pragma mark - Subscribe characteristic

    - (void)helperWillSubscribeCharacteristic:(id)serviceRef {
        // Write／Notifyキャラクタリスティックの参照を保持
        [self setCharacteristicForWrite:nil];
        [self setCharacteristicForWriteNoResp:nil];
        [self setCharacteristicForNotify:nil];
        CBService *connectedService = (CBService *)serviceRef;
        for (CBCharacteristic *characteristic in [connectedService characteristics]) {
            if ([characteristic properties] & CBCharacteristicPropertyWrite) {
                [self setCharacteristicForWrite:characteristic];
            }
            if ([characteristic properties] & CBCharacteristicPropertyWriteWithoutResponse) {
                [self setCharacteristicForWriteNoResp:characteristic];
            }
            if ([characteristic properties] & CBCharacteristicPropertyNotify) {
                [self setCharacteristicForNotify:characteristic];
            }
        }
        if ([self characteristicForNotify]) {
            // Notifyキャラクタリスティックに対する監視を開始
            [[self discoveredPeripheral] setNotifyValue:YES forCharacteristic:[self characteristicForNotify]];
        }
    }

    - (void)peripheral:(CBPeripheral *)peripheral didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic
                 error:(NSError *)error {
        // 監視開始エラー発生の場合
        if (error) {
            // 監視開始エラー発生を通知
            [[self delegate] helperDidFailConnectionWithError:error reason:BLE_ERR_NOTIFICATION_FAILED];
            return;
        }
        if ([characteristic isNotifying]) {
            // 監視開始を通知
            [[ToolLogFile defaultLogger] debug:@"BLE connection has started successfully."];
            [[self delegate] helperDidSubscribeCharacteristic];
        } else {
            // 監視が停止している場合は通知
            [[self delegate] helperDidFailConnectionWithError:nil reason:BLE_ERR_NOTIFICATION_STOP];
        }
    }

    - (bool)helperIsSubscribingCharacteristic {
        if ([self discoveredPeripheral] == nil) {
            return false;
        }
        if ([self characteristicForNotify] == nil) {
            return false;
        }
        return [[self characteristicForNotify] isNotifying];
    }

#pragma mark - Write value for characteristics

    - (void)helperWillWriteForCharacteristics:(NSData *)requestMessage {
        // Writeキャラクタリスティックへの書き込みを開始
        if ([self characteristicForWrite]) {
            [[self discoveredPeripheral] writeValue:requestMessage
                                 forCharacteristic:[self characteristicForWrite]
                                              type:CBCharacteristicWriteWithResponse];
        } else if ([self characteristicForWriteNoResp]) {
            [[self discoveredPeripheral] writeValue:requestMessage
                                 forCharacteristic:[self characteristicForWriteNoResp]
                                              type:CBCharacteristicWriteWithoutResponse];
            [[self delegate] helperDidWriteForCharacteristics];
        }
    }

    - (void)peripheral:(CBPeripheral *)peripheral didWriteValueForCharacteristic:(CBCharacteristic *)characteristic
                 error:(NSError *)error {
        // Writeキャラクタリスティック書込エラー発生の場合は通知
        if (error) {
            [[self delegate] helperDidFailConnectionWithError:error reason:BLE_ERR_REQUEST_SEND_FAILED];
            return;
        }
        // 書込み完了を通知
        if ([self characteristicForWrite]) {
            [[self delegate] helperDidWriteForCharacteristics];
        }
    }

#pragma mark - Read value for characteristics

    - (void)helperWillReadForCharacteristics {
        // Notifyキャラクタリスティック経由のレスポンス待ち（レスポンスタイムアウト監視開始）
        [self startResponseTimeoutMonitor:[self characteristicForNotify]];
    }

    - (void)peripheral:(CBPeripheral *)peripheral didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic
                 error:(NSError *)error {
        // レスポンスタイムアウト監視を停止
        [self cancelResponseTimeoutMonitor:[self characteristicForNotify]];
        // Notifyキャラクタリスティックからデータ取得時にエラー発生の場合通知
        if (error) {
            [[self delegate] helperDidFailConnectionWithError:error reason:BLE_ERR_RESPONSE_RECEIVE_FAILED];
            return;
        }
        // 受信データを転送
        [[self delegate] helperDidReadForCharacteristic:[characteristic value]];
    }

    - (void)responseDidTimeout {
        // レスポンスタイムアウト発生の場合は通知
        [[self delegate] helperDidFailConnectionWithError:nil reason:BLE_ERR_REQUEST_TIMEOUT];
    }

#pragma mark - Disconnect from peripheral

    - (void)helperWillDisconnect {
        // ペリフェラル接続を切断
        if ([self discoveredPeripheral] != nil) {
            [[self manager] cancelPeripheralConnection:[self discoveredPeripheral]];
        } else {
            [[self delegate] helperDidDisconnectWithError:nil peripheral:nil];
        }
    }

    - (void)helperWillDisconnectForce:(id)peripheralRef {
        // ペリフェラル接続を切断
        if (peripheralRef) {
            [[self manager] cancelPeripheralConnection:(CBPeripheral *)peripheralRef];
        } else {
            [[self delegate] helperDidDisconnectWithError:nil peripheral:nil];
        }
    }

#pragma mark - Timeout monitor

    - (void)startTimeoutMonitorForSelector:(SEL)selector withObject:object afterDelay:(NSTimeInterval)delay {
        [self cancelTimeoutMonitorForSelector:selector withObject:object];
        [self performSelector:selector withObject:object afterDelay:delay];
    }

    - (void)cancelTimeoutMonitorForSelector:(SEL)selector withObject:object {
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:selector object:object];
    }

#pragma mark - Scanning Timeout Monitor

    - (void)startScanningTimeoutMonitor {
        // スキャンタイムアウト監視を開始（10秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(scanningTimeoutMonitorDidTimeout)
                                  withObject:nil afterDelay:10.0];
    }

    - (void)cancelScanningTimeoutMonitor {
        // スキャンタイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(scanningTimeoutMonitorDidTimeout)
                                   withObject:nil];
    }

    - (void)scanningTimeoutMonitorDidTimeout {
        // スキャンタイムアウト時の処理を実行
        [self scanningDidTimeout];
    }

#pragma mark - Response Timeout Monitor

    - (void)startResponseTimeoutMonitor:(CBCharacteristic *)characteristic {
        // レスポンスタイムアウト監視を開始（10秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(responseTimeoutMonitorDidTimeout)
                                  withObject:characteristic afterDelay:10.0];
    }

    - (void)cancelResponseTimeoutMonitor:(CBCharacteristic *)characteristic {
        // レスポンスタイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(responseTimeoutMonitorDidTimeout)
                                   withObject:characteristic];
    }

    - (void)responseTimeoutMonitorDidTimeout {
        // レスポンスタイムアウト時の処理を実行
        [self responseDidTimeout];
    }

@end
