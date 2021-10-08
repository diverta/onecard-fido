//
//  ToolBLEHelper.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/07.
//
#import "ToolBLEHelper.h"
#import "ToolCommonMessage.h"

@interface ToolBLEHelper () <CBCentralManagerDelegate, CBPeripheralDelegate>

    @property(nonatomic, weak) id<ToolBLEHelperDelegate> delegate;
    @property(nonatomic) CBCentralManager   *manager;
    @property(nonatomic) CBPeripheral       *connectedPeripheral;
    @property(nonatomic) CBService          *connectedService;
    @property(nonatomic) CBCharacteristic   *characteristicForWrite;
    @property(nonatomic) CBCharacteristic   *characteristicForNotify;;
    @property(nonatomic, strong) NSArray    *serviceUUIDs;
    @property(nonatomic, strong) NSArray    *characteristicUUIDs;

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
            [self setConnectedPeripheral:nil];
        }
        return self;
    }

    - (void)centralManagerDidUpdateState:(CBCentralManager *)central {
        CBCentralManagerState _state = (CBCentralManagerState)[central state];
        [[self delegate] notifyCentralManagerStateUpdate:_state];
    }

#pragma mark - Entry for process

    - (void)helperWillConnectWithUUID:(NSString *)uuidString {
        // すでに接続が確立されている場合は通知
        if ([self connectedPeripheral] != nil) {
            [[self delegate] helperDidConnectPeripheral];
            return;
        }
        // BLEが無効化されている場合は通知
        if ([[self manager] state] != CBCentralManagerStatePoweredOn) {
            [[self delegate] helperDidFailConnectionWith:BLE_ERR_BLUETOOTH_OFF error:nil];
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
        [self setConnectedPeripheral:nil];
        [[self manager] scanForPeripheralsWithServices:nil options:scanningOptions];
        [[self delegate] helperNotifyStatus:BLE_ERR_DEVICE_SCAN_START error:nil];
        // スキャンタイムアウト監視を開始
        [self startScanningTimeoutMonitor];
    }

    - (void)cancelScanForPeripherals {
        // スキャンを停止
        [[self manager] stopScan];
        [[self delegate] helperNotifyStatus:BLE_ERR_DEVICE_SCAN_STOPPED error:nil];
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
            // スキャンタイムアウト監視を停止
            [self cancelScanningTimeoutMonitor];
            // スキャンを停止し、ペリフェラルに接続
            [self cancelScanForPeripherals];
            [self connectPeripheral:peripheral];
            break;
        }
    }

    - (void)scanningDidTimeout {
        // スキャンを停止
        [self cancelScanForPeripherals];
        // スキャンタイムアウトの場合は通知
        [[self delegate] helperDidFailConnectionWith:BLE_ERR_DEVICE_SCAN_TIMEOUT error:nil];
    }

#pragma mark - Connect peripheral

    - (void)connectPeripheral:(CBPeripheral *)peripheral {
        // ペリフェラルに接続し、接続タイムアウト監視を開始
        [[self manager] connectPeripheral:peripheral options:nil];
        [self startConnectionTimeoutMonitor:peripheral];
    }

    - (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral {
        // 接続タイムアウト監視を停止
        [self cancelConnectionTimeoutMonitor:peripheral];
        // すでに接続されている状態の場合は終了
        if ([self connectedPeripheral] != nil) {
            return;
        }
        // 接続されたペリフェラルの参照を保持
        [self setConnectedPeripheral:peripheral];
        // 接続完了を通知
        [[self delegate] helperDidConnectPeripheral];
    }

    - (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)peripheral
                     error:(NSError *)error {
        // 接続タイムアウト監視を停止
        [self cancelConnectionTimeoutMonitor:peripheral];
        // 接続失敗を通知
        [[self delegate] helperDidFailConnectionWith:BLE_ERR_DEVICE_CONNECT_FAILED error:error];
    }

    - (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)peripheral
                     error:(NSError *)error {
        // ペリフェラルの参照を解除
        [self setConnectedPeripheral:nil];
        // ログをファイル出力
        [[self delegate] helperNotifyStatus:BLE_ERR_DEVICE_DISCONNECTED error:error];
        // 切断完了を通知
        [[self delegate] helperDidDisconnect];
    }

    - (void)connectionDidTimeout {
        // 接続タイムアウトを通知
        [[self delegate] helperDidFailConnectionWith:BLE_ERR_DEVICE_CONNREQ_TIMEOUT error:nil];
    }

#pragma mark - Discover services

    - (void)helperWillDiscoverServiceWithUUID:(NSString *)uuidString {
        // サービスのディスカバーを開始
        CBPeripheral *peripheral = [self connectedPeripheral];
        [peripheral setDelegate:self];
        [peripheral discoverServices:nil];
        [self setServiceUUIDs:@[[CBUUID UUIDWithString:uuidString]]];
        // サービス・ディスカバーのタイムアウト監視を開始
        [self startDiscoverServicesTimeoutMonitor:peripheral];
    }

    - (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error {
        // サービス・ディスカバーのタイムアウト監視を停止
        [self cancelDiscoverServicesTimeoutMonitor:peripheral];
        // BLEサービスディスカバーに失敗の場合は通知
        if (error) {
            [[self delegate] helperDidFailConnectionWith:BLE_ERR_SERVICE_NOT_DISCOVERED error:error];
            return;
        }
        // サービスを判定し、その参照を保持
        [self setConnectedService:nil];
        for (CBService *service in [peripheral services]) {
            if ([[self serviceUUIDs] containsObject:service.UUID]) {
                [self setConnectedService:service];
                break;
            }
        }
        // サービスがない場合は通知
        if ([self connectedService] == nil) {
            [[self delegate] helperDidFailConnectionWith:BLE_ERR_SERVICE_NOT_FOUND error:nil];
            return;
        }
        // ディスカバー完了を通知
        [[self delegate] helperDidDiscoverService];
    }

    - (void)discoverServicesDidTimeout {
        // サービスディスカバータイムアウトの場合は通知
        [[self delegate] helperDidFailConnectionWith:BLE_ERR_DISCOVER_SERVICE_TIMEOUT error:nil];
    }

#pragma mark - Discover characteristics

    - (void)helperWillDiscoverCharacteristicsWithUUIDs:(NSArray<NSString *> *)uuids {
        // ディスカバー対象のキャラクタリスティックUUIDを保持
        NSMutableArray *array = [[NSMutableArray alloc] init];
        for (NSString *uuidString in uuids) {
            [array addObject:[CBUUID UUIDWithString:uuidString]];
        }
        [self setCharacteristicUUIDs:array];
        // サービス内のキャラクタリスティックをディスカバー
        [[self connectedPeripheral] discoverCharacteristics:[self characteristicUUIDs] forService:[self connectedService]];
        // キャラクタリスティック・ディスカバーのタイムアウト監視を開始
        [self startDiscoverCharacteristicsTimeoutMonitor:[self connectedService]];
    }

    - (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service
            error:(NSError *)error {
        // キャラクタリスティック・ディスカバーのタイムアウト監視を停止
        [self cancelDiscoverCharacteristicsTimeoutMonitor:service];
        // キャラクタリスティックのディスカバーエラー発生の場合は通知
        if (error) {
            [[self delegate] helperDidFailConnectionWith:BLE_ERR_CHARACT_NOT_DISCOVERED error:error];
            return;
        }
        // Notifyキャラクタリスティックに対する監視を開始
        [self subscribeCharacteristic:service];
    }

    - (void)discoverCharacteristicsDidTimeout {
        // キャラクタリスティック・ディスカバータイムアウトの場合は通知
        [[self delegate] helperDidFailConnectionWith:BLE_ERR_DISCOVER_CHARACT_TIMEOUT error:nil];
    }

#pragma mark - Subscribe characteristic

    - (void)subscribeCharacteristic:(CBService *)service {
        // サービスにキャラクタリスティックがない場合は通知
        if ([[service characteristics] count] < 1) {
            [[self delegate] helperDidFailConnectionWith:BLE_ERR_CHARACT_NOT_EXIST error:nil];
            return;
        }
        // Write／Notifyキャラクタリスティックの参照を保持
        [self setCharacteristicForWrite:nil];
        [self setCharacteristicForNotify:nil];
        for (CBCharacteristic *characteristic in [service characteristics]) {
            if ([characteristic properties] & CBCharacteristicPropertyWrite) {
                [self setCharacteristicForWrite:characteristic];
            } else if ([characteristic properties] & CBCharacteristicPropertyNotify) {
                [self setCharacteristicForNotify:characteristic];
            }
        }
        // Notifyキャラクタリスティックに対する監視を開始
        if ([self characteristicForNotify]) {
            [[self connectedPeripheral] setNotifyValue:YES forCharacteristic:[self characteristicForNotify]];
        }
        // 監視ステータス更新のタイムアウト監視を開始
        [self startSubscribeCharacteristicTimeoutMonitor:[self characteristicForNotify]];
    }

    - (void)peripheral:(CBPeripheral *)peripheral didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic
                 error:(NSError *)error {
        // 監視ステータス更新のタイムアウト監視を停止
        [self cancelSubscribeCharacteristicTimeoutMonitor:characteristic];
        // 監視開始エラー発生の場合は通知
        if (error) {
            [[self delegate] helperDidFailConnectionWith:BLE_ERR_NOTIFICATION_FAILED error:error];
            return;
        }
        if ([characteristic isNotifying]) {
            // 監視開始の場合は、ディスカバー完了を通知
            [[self delegate] helperDidDiscoverCharacteristics];
        } else {
            // 監視が停止している場合は通知
            [[self delegate] helperDidFailConnectionWith:BLE_ERR_NOTIFICATION_STOP error:nil];
        }
    }

    - (void)subscribeCharacteristicDidTimeout {
        // 監視ステータス更新タイムアウトの旨をAppDelegateに通知
        [[self delegate] helperDidFailConnectionWith:BLE_ERR_SUBSCRIBE_CHARACT_TIMEOUT error:nil];
    }

#pragma mark - Write value for characteristics

    - (void)helperWillWriteForCharacteristics:(NSData *)requestMessage {
        // Writeキャラクタリスティックへの書き込みを開始
        [[self connectedPeripheral] writeValue:requestMessage
                             forCharacteristic:[self characteristicForWrite]
                                          type:CBCharacteristicWriteWithResponse];
    }

    - (void)peripheral:(CBPeripheral *)peripheral didWriteValueForCharacteristic:(CBCharacteristic *)characteristic
                 error:(NSError *)error {
        // Writeキャラクタリスティック書込エラー発生の場合は通知
        if (error) {
            [[self delegate] helperDidFailConnectionWith:BLE_ERR_REQUEST_SEND_FAILED error:error];
            return;
        }
        // 書込み完了を通知
        [[self delegate] helperDidWriteForCharacteristics];
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
            [[self delegate] helperDidFailConnectionWith:BLE_ERR_RESPONSE_RECEIVE_FAILED error:error];
            return;
        }
        // 受信データを転送
        [[self delegate] helperDidReadForCharacteristic:[characteristic value]];
    }

    - (void)responseDidTimeout {
        // レスポンスタイムアウト発生の場合は通知
        [[self delegate] helperDidFailConnectionWith:BLE_ERR_REQUEST_TIMEOUT error:nil];
    }

#pragma mark - Disconnect from peripheral

    - (void)helperWillDisconnect {
        // ペリフェラル接続を切断
        if ([self connectedPeripheral] != nil) {
            [[self manager] cancelPeripheralConnection:[self connectedPeripheral]];
        } else {
            [[self delegate] helperDidDisconnect];
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

#pragma mark - Connecting Timeout Monitor

    - (void)startConnectionTimeoutMonitor:(CBPeripheral *)peripheral {
        // 接続タイムアウト監視を開始（10秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(connectionTimeoutMonitorDidTimeout)
                                  withObject:peripheral afterDelay:10.0];
    }

    - (void)cancelConnectionTimeoutMonitor:(CBPeripheral *)peripheral {
        // 接続タイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(connectionTimeoutMonitorDidTimeout)
                                   withObject:peripheral];
    }

    - (void)connectionTimeoutMonitorDidTimeout {
        // 接続タイムアウト時の処理を実行
        [self connectionDidTimeout];
    }

#pragma mark - Discover Services Timeout Monitor

    - (void)startDiscoverServicesTimeoutMonitor:(CBPeripheral *)peripheral {
        // サービスディスカバータイムアウト監視を開始（10秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(discoverServicesTimeoutMonitorDidTimeout)
                                  withObject:peripheral afterDelay:10.0];
    }

    - (void)cancelDiscoverServicesTimeoutMonitor:(CBPeripheral *)peripheral {
        // サービスディスカバータイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(discoverServicesTimeoutMonitorDidTimeout)
                                   withObject:peripheral];
    }

    - (void)discoverServicesTimeoutMonitorDidTimeout {
        // サービスディスカバータイムアウト時の処理を実行
        [self discoverServicesDidTimeout];
    }

#pragma mark - Discover Characteristics Timeout Monitor

    - (void)startDiscoverCharacteristicsTimeoutMonitor:(CBService *)service {
        // サービスディスカバータイムアウト監視を開始（10秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(discoverCharacteristicsTimeoutMonitorDidTimeout)
                                  withObject:service afterDelay:10.0];
    }

    - (void)cancelDiscoverCharacteristicsTimeoutMonitor:(CBService *)service {
        // サービスディスカバータイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(discoverCharacteristicsTimeoutMonitorDidTimeout)
                                   withObject:service];
    }

    - (void)discoverCharacteristicsTimeoutMonitorDidTimeout {
        // キャラクタリスティック・ディスカバーのタイムアウト時の処理を実行
        [self discoverCharacteristicsDidTimeout];
    }

#pragma mark - Subscribe Characteristic Timeout Monitor

    - (void)startSubscribeCharacteristicTimeoutMonitor:(CBCharacteristic *)characteristic {
        // 監視ステータス更新タイムアウト監視を開始（10秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(subscribeCharacteristicTimeoutMonitorDidTimeout)
                                  withObject:characteristic afterDelay:10.0];
    }

    - (void)cancelSubscribeCharacteristicTimeoutMonitor:(CBCharacteristic *)characteristic {
        // 監視ステータス更新タイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(subscribeCharacteristicTimeoutMonitorDidTimeout)
                                   withObject:characteristic];
    }

    - (void)subscribeCharacteristicTimeoutMonitorDidTimeout {
        // 監視ステータス更新タイムアウト時の処理を実行
        [self subscribeCharacteristicDidTimeout];
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
