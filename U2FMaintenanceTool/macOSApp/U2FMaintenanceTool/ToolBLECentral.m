#import "ToolBLECentral.h"
#import "ToolCommonMessage.h"

static const NSTimeInterval kScanningTimeout   = 10.0;
static const NSTimeInterval kConnectingTimeout = 10.0;
static const NSTimeInterval kRequestTimeout    = 20.0;

#define U2FServiceUUID          @"0000FFFD-0000-1000-8000-00805F9B34FB"
#define U2FControlPointCharUUID @"F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB"
#define U2FStatusCharUUID       @"F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB"

@interface ToolBLECentral () <CBCentralManagerDelegate, CBPeripheralDelegate>

    @property(nonatomic) CBCentralManager *manager;
    @property(nonatomic) CBPeripheral     *connectedPeripheral;
    @property(nonatomic) CBService        *connectedService;

    @property(nonatomic) CBCharacteristic *u2fControlPointChar;
    @property(nonatomic) CBCharacteristic *u2fStatusChar;

@end

@implementation ToolBLECentral

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolBLECentralDelegate>)delegate {
        self = [super init];
        if (self) {
            self.delegate = delegate;
            self.manager = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
            self.connectedPeripheral = nil;
            self.serviceUUIDs = @[[CBUUID UUIDWithString:U2FServiceUUID]];
            self.characteristicUUIDs = @[[CBUUID UUIDWithString:U2FControlPointCharUUID],
                                         [CBUUID UUIDWithString:U2FStatusCharUUID]];
        }
        return self;
    }

    - (void)centralManagerDidUpdateState:(CBCentralManager *)central {
        CBCentralManagerState _state = (CBCentralManagerState)[central state];
        [self.delegate notifyCentralManagerStateUpdate:_state];
    }

#pragma mark - Entry for process

    - (void)centralManagerWillConnect {
        if (self.connectedPeripheral) {
            // すでに接続が確立されている場合はAppDelegateに通知
            [self.delegate centralManagerDidConnect];
            return;
        }

        NSAssert(self.serviceUUIDs.count > 0, @"Need to specify services");
        NSAssert(self.characteristicUUIDs.count > 0, @"Need to specify characteristics UUID");

        if (self.manager.state != CBCentralManagerStatePoweredOn) {
            // BLEが無効化されている旨をAppDelegateに通知
            [[self delegate] notifyCentralManagerErrorMessage:MSG_INVALID_BLE_PERIPHERAL
                                                        error:nil];
            [[self delegate] centralManagerDidFailConnection];
            return;
        }

        // FIDO BLE U2Fサービスを有するペリフェラルのスキャン開始
        [self scanForPeripherals];
    }

#pragma mark - Scan for peripherals

    - (void)scanForPeripherals {
        // スキャン設定
        [self startScanningTimeoutMonitor];
        NSDictionary *scanningOptions = @{CBCentralManagerScanOptionAllowDuplicatesKey : @NO};

        // FIDO BLE U2Fサービスを持つペリフェラルをスキャン
        self.connectedPeripheral = nil;
        [self.manager scanForPeripheralsWithServices:nil options:scanningOptions];
        [[self delegate] notifyCentralManagerMessage:MSG_U2F_DEVICE_SCAN_START];
    }

    - (void)cancelScanForPeripherals {
        [self.manager stopScan];
    }

    - (void)centralManager:(CBCentralManager *)central
     didDiscoverPeripheral:(CBPeripheral *)peripheral
         advertisementData:(NSDictionary *)advertisementData
                      RSSI:(NSNumber *)RSSI {
        // FIDO BLE U2Fサービスを有するペリフェラルかどうか判定
        NSArray *serviceUUIDs = [advertisementData objectForKey:CBAdvertisementDataServiceUUIDsKey];
        for (CBUUID *foundServiceUUIDs in serviceUUIDs) {
            if (![self.serviceUUIDs containsObject:foundServiceUUIDs]) {
                continue;
            }
            // スキャンを停止し、ペリフェラルに接続
            [self cancelScanForPeripherals];
            [self cancelScanningTimeoutMonitor];
            [self connectPeripheral:peripheral];
            [[self delegate] notifyCentralManagerMessage:MSG_U2F_DEVICE_SCAN_END];
            break;
        }
    }

#pragma mark - Scanning timeout monitor

    - (void)startScanningTimeoutMonitor {
        [self cancelScanningTimeoutMonitor];
        [self performSelector:@selector(scanningDidTimeout) withObject:nil afterDelay:kScanningTimeout];
    }

    - (void)cancelScanningTimeoutMonitor {
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(scanningDidTimeout) object:nil];
    }

    - (void)scanningDidTimeout {
        // スキャンタイムアウトの旨をAppDelegateに通知
        [[self delegate] notifyCentralManagerErrorMessage:MSG_U2F_DEVICE_SCAN_TIMEOUT
                                                    error:nil];
        [[self delegate] centralManagerDidFailConnection];
    }

#pragma mark - Connection timeout monitor

    - (void)startConnectionTimeoutMonitor:(CBPeripheral *)peripheral {
        [self cancelConnectionTimeoutMonitor:peripheral];
        [self performSelector:@selector(connectionDidTimeout:)
                   withObject:peripheral
                   afterDelay:kConnectingTimeout];
    }

    - (void)cancelConnectionTimeoutMonitor:(CBPeripheral *)peripheral {
        [NSObject cancelPreviousPerformRequestsWithTarget:self
                                                 selector:@selector(connectionDidTimeout:)
                                                   object:peripheral];
    }

    - (void)connectionDidTimeout:(CBPeripheral *)peripheral {
        // 接続タイムアウトの旨をAppDelegateに通知
        [[self delegate] notifyCentralManagerErrorMessage:MSG_U2F_DEVICE_CONNREQ_TIMEOUT
                                                    error:nil];
        [[self delegate] centralManagerDidFailConnection];
    }

#pragma mark - Connect peripheral

    - (void)connectPeripheral:(CBPeripheral *)peripheral {
        // ペリフェラルに接続し、接続タイムアウト監視を開始
        [self.manager connectPeripheral:peripheral options:nil];
        [self startConnectionTimeoutMonitor:peripheral];
    }

    - (void)centralManager:(CBCentralManager *)central
            didConnectPeripheral:(CBPeripheral *)peripheral {
        // 接続タイムアウト監視を停止
        [self cancelConnectionTimeoutMonitor:peripheral];
        // すでに接続されている状態の場合は終了
        if (self.connectedPeripheral) {
            NSLog(@"didConnectPeripheral: already connected to peripheral");
            return;
        }
        // 接続されたペリフェラルの参照を保持
        self.connectedPeripheral = peripheral;
        [[self delegate] notifyCentralManagerMessage:MSG_U2F_DEVICE_CONNECTED];

        // FIDO BLE U2Fサービスのディスカバーを開始
        [self discoverServices:peripheral];
    }

    - (void)centralManager:(CBCentralManager *)central
didFailToConnectPeripheral:(CBPeripheral *)peripheral
                     error:(NSError *)error {
        // 接続タイムアウト監視を停止
        [self cancelConnectionTimeoutMonitor:peripheral];
        // 接続失敗の旨をAppDelegateに通知
        [[self delegate] notifyCentralManagerErrorMessage:MSG_U2F_DEVICE_CONNECT_FAILED
                                                    error:error];
        [[self delegate] centralManagerDidFailConnection];
    }

    - (void)centralManager:(CBCentralManager *)central
   didDisconnectPeripheral:(CBPeripheral *)peripheral
                     error:(NSError *)error {
        // ペリフェラル、サービス、キャラクタリスティックの参照を解除
        self.connectedPeripheral = nil;
        self.connectedService    = nil;
        self.u2fControlPointChar = nil;
        self.u2fStatusChar       = nil;
        // 切断完了
        [[self delegate] notifyCentralManagerErrorMessage:MSG_U2F_DEVICE_DISCONNECTED
                                                    error:error];
        [[self delegate] centralManagerDidDisconnect];
    }

#pragma mark - Discover services

    - (void)discoverServices:(CBPeripheral *)peripheral {
        // FIDO BLE U2Fサービスのディスカバーを開始
        [peripheral setDelegate:self];
        [peripheral discoverServices:nil];
    }

    - (void)peripheral:(CBPeripheral *)peripheral
   didDiscoverServices:(NSError *)error {
        if (error) {
            // BLEサービスディスカバーに失敗の旨をAppDelegateに通知
            [[self delegate] notifyCentralManagerErrorMessage:MSG_BLE_SERVICE_NOT_DISCOVERED
                                                        error:error];
            [[self delegate] centralManagerDidFailConnection];
            return;
        }

        // FIDO BLE U2Fサービスを判定し、その参照を保持
        self.connectedService = nil;
        for (CBService *service in peripheral.services) {
            if ([self.serviceUUIDs containsObject:service.UUID]) {
                self.connectedService = service;
                [[self delegate] notifyCentralManagerMessage:MSG_BLE_U2F_SERVICE_FOUND];
                break;
            }
        }

        if (!self.connectedService) {
            // FIDO BLE U2Fサービスがない旨をAppDelegateに通知
            [[self delegate] notifyCentralManagerErrorMessage:MSG_BLE_U2F_SERVICE_NOT_FOUND
                                                        error:nil];
            [[self delegate] centralManagerDidFailConnection];
            return;
        }

        // キャラクタリスティックのディスカバーを実行
        [self discoverServiceCharacteristics:self.connectedService];
    }

#pragma mark - Discover characteristics

    - (void)discoverServiceCharacteristics:(CBService *)service {
        // サービス内のキャラクタリスティックをディスカバー
        [self.connectedPeripheral discoverCharacteristics:self.characteristicUUIDs
                                               forService:service];
    }

    - (void)peripheral:(CBPeripheral *)peripheral
            didDiscoverCharacteristicsForService:(CBService *)service
            error:(NSError *)error {
        if (error) {
            // キャラクタリスティックのディスカバーエラー発生の旨をAppDelegateに通知
            [[self delegate] notifyCentralManagerErrorMessage:MSG_BLE_CHARACT_NOT_DISCOVERED
                                                        error:error];
            [[self delegate] centralManagerDidFailConnection];
            return;
        }

        // U2F Statusキャラクタリスティックに対する監視を開始
        [self subscribeCharacteristic:service];
    }

#pragma mark - Subscribe characteristic

    - (void)subscribeCharacteristic:(CBService *)service {
        // サービスの参照を保持
        self.connectedService = service;

        if (service.characteristics.count < 1) {
            // サービスにキャラクタリスティックがない旨をAppDelegateに通知
            [[self delegate] notifyCentralManagerErrorMessage:MSG_BLE_CHARACT_NOT_EXIST
                                                        error:nil];
            [[self delegate] centralManagerDidFailConnection];
            return;
        }

        // FIDO U2Fキャラクタリスティックの参照を保持
        self.u2fControlPointChar = nil;
        self.u2fStatusChar       = nil;
        for (CBCharacteristic *characteristic in service.characteristics) {
            if (characteristic.properties & CBCharacteristicPropertyWrite) {
                self.u2fControlPointChar = characteristic;
            } else if (characteristic.properties & CBCharacteristicPropertyNotify) {
                self.u2fStatusChar = characteristic;
            }
        }

        // U2F Statusキャラクタリスティックに対する監視を開始
        if (self.u2fStatusChar) {
            [self.connectedPeripheral setNotifyValue:YES forCharacteristic:self.u2fStatusChar];
        }
    }

    - (void)peripheral:(CBPeripheral *)peripheral
        didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic
        error:(NSError *)error {
        if (error) {
            // U2F Status監視開始エラー発生の旨をAppDelegateに通知
            [[self delegate] notifyCentralManagerErrorMessage:MSG_BLE_NOTIFICATION_FAILED
                                                        error:error];
            [[self delegate] centralManagerDidFailConnection];
            return;
        }

        if (characteristic.isNotifying) {
            // 一連の接続処理が完了したことをAppDelegateに通知
            [[self delegate] notifyCentralManagerMessage:MSG_BLE_NOTIFICATION_START];
            [self.delegate centralManagerDidConnect];
        } else {
            // 監視が停止している旨をAppDelegateに通知
            [[self delegate] notifyCentralManagerErrorMessage:MSG_BLE_NOTIFICATION_STOP
                                                        error:nil];
            [[self delegate] centralManagerDidFailConnection];

        }
    }

#pragma mark - Do main process

    - (void)centralManagerWillSend:(NSArray<NSData *> *)bleMessages {
        // U2F Control Pointに、実行するコマンドを書き込み
        for (NSData *data in bleMessages) {
            [NSThread sleepForTimeInterval:0.25];
            [self.connectedPeripheral writeValue:data
                               forCharacteristic:self.u2fControlPointChar
                                            type:CBCharacteristicWriteWithResponse];
            NSLog(@"Sent request %@", data);
        }
        [[self delegate] notifyCentralManagerMessage:MSG_REQUEST_SENT];
    }

#pragma mark - Request timeout monitor

    - (void)centralManagerWillStartResponseTimeout {
        // U2F Status経由のレスポンス待ち（タイムアウト監視開始）
        [self startRequestTimeout:self.u2fStatusChar];
    }

    - (void)startRequestTimeout:(CBCharacteristic *)characteristic {
        [self cancelRequestTimeoutMonitor:characteristic];
        [self performSelector:@selector(requestDidTimeout:)
            withObject:characteristic afterDelay:kRequestTimeout];
    }

    - (void)cancelRequestTimeoutMonitor:(CBCharacteristic *)characteristic {
        [NSObject cancelPreviousPerformRequestsWithTarget:self
            selector:@selector(requestDidTimeout:) object:characteristic];
    }

    - (void)requestDidTimeout:(CBCharacteristic *)characteristic {
        // リクエストタイムアウト発生の旨をAppDelegateに通知
        [[self delegate] notifyCentralManagerErrorMessage:MSG_REQUEST_TIMEOUT
                                                    error:nil];
        [[self delegate] centralManagerDidFailConnection];
    }

#pragma mark - Write value for characteristics

    - (void)peripheral:(CBPeripheral *)peripheral
            didWriteValueForCharacteristic:(CBCharacteristic *)characteristic
            error:(NSError *)error {
        if (error) {
            // U2F Control Point書込エラー発生の旨をAppDelegateに通知
            [[self delegate] notifyCentralManagerErrorMessage:MSG_REQUEST_SEND_FAILED
                                                        error:error];
            [[self delegate] centralManagerDidFailConnection];
            return;
        }

        // U2F Status経由のレスポンス待ち（タイムアウト監視開始）
        [self centralManagerWillStartResponseTimeout];
    }

    - (void)peripheral:(CBPeripheral *)peripheral
            didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic
            error:(NSError *)error {
        // タイムアウト監視を停止
        [self cancelRequestTimeoutMonitor:self.u2fStatusChar];

        if (error) {
            // U2F Status取得エラー発生の旨をAppDelegateに通知
            [[self delegate] notifyCentralManagerErrorMessage:MSG_RESPONSE_RECEIVE_FAILED
                                                        error:error];
            [[self delegate] centralManagerDidFailConnection];
            return;
        }

        // 受信データをAppDelegateへ転送
        [self.delegate centralManagerDidReceive:[characteristic value]];
    }

#pragma mark - Disconnect from peripheral

    - (void)centralManagerWillDisconnect {
        if (self.connectedPeripheral) {
            // スキャンを停止し、ペリフェラル接続を切断
            [self cancelScanForPeripherals];
            [self.manager cancelPeripheralConnection:self.connectedPeripheral];
        } else {
            [[self delegate] centralManagerDidDisconnect];
        }
    }

@end
