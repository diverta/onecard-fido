#import "ToolBLECentral.h"
#import "ToolCommonMessage.h"
#import "ToolTimer.h"

static const NSTimeInterval kRequestTimeout    = 20.0;

#define U2FServiceUUID          @"0000FFFD-0000-1000-8000-00805F9B34FB"
#define U2FControlPointCharUUID @"F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB"
#define U2FStatusCharUUID       @"F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB"

@interface ToolBLECentral () <CBCentralManagerDelegate, CBPeripheralDelegate, ToolTimerDelegate>

    @property(nonatomic) CBCentralManager *manager;
    @property(nonatomic) CBPeripheral     *connectedPeripheral;
    @property(nonatomic) CBService        *connectedService;

    @property(nonatomic) CBCharacteristic *u2fControlPointChar;
    @property(nonatomic) CBCharacteristic *u2fStatusChar;

    // 送信フレームデータ／フレーム数を保持
    @property(nonatomic) NSArray<NSData *> *bleRequestFrames;
    @property(nonatomic) NSUInteger         bleRequestFrameNumber;

    // タイムアウトモニター
    @property(nonatomic) ToolTimer         *toolTimer;

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
            self.toolTimer = [[ToolTimer alloc] initWithDelegate:self];
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
        NSDictionary *scanningOptions = @{CBCentralManagerScanOptionAllowDuplicatesKey : @NO};

        // FIDO BLE U2Fサービスを持つペリフェラルをスキャン
        self.connectedPeripheral = nil;
        [self.manager scanForPeripheralsWithServices:nil options:scanningOptions];
        [[self delegate] notifyCentralManagerMessage:MSG_U2F_DEVICE_SCAN_START];
        
        // スキャンタイムアウト監視を開始
        [[self toolTimer] startScanningTimeoutMonitor];
    }

    - (void)cancelScanForPeripherals {
        // スキャンを停止
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
            // スキャンタイムアウト監視を停止
            [[self toolTimer] cancelScanningTimeoutMonitor];
            // スキャンを停止し、ペリフェラルに接続
            [self cancelScanForPeripherals];
            [self connectPeripheral:peripheral];
            [[self delegate] notifyCentralManagerMessage:MSG_U2F_DEVICE_SCAN_END];
            break;
        }
    }

    - (void)scanningDidTimeout {
        // スキャンタイムアウトの旨をAppDelegateに通知
        [[self delegate] notifyCentralManagerErrorMessage:MSG_U2F_DEVICE_SCAN_TIMEOUT
                                                    error:nil];
        [[self delegate] centralManagerDidFailConnection];
    }

    - (void)connectionDidTimeout {
        // 接続タイムアウトの旨をAppDelegateに通知
        [[self delegate] notifyCentralManagerErrorMessage:MSG_U2F_DEVICE_CONNREQ_TIMEOUT
                                                    error:nil];
        [[self delegate] centralManagerDidFailConnection];
    }

#pragma mark - Connect peripheral

    - (void)connectPeripheral:(CBPeripheral *)peripheral {
        // ペリフェラルに接続し、接続タイムアウト監視を開始
        [self.manager connectPeripheral:peripheral options:nil];
        [[self toolTimer] startConnectionTimeoutMonitor:peripheral];
    }

    - (void)centralManager:(CBCentralManager *)central
            didConnectPeripheral:(CBPeripheral *)peripheral {
        // 接続タイムアウト監視を停止
        [[self toolTimer] cancelConnectionTimeoutMonitor:peripheral];
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
        [[self toolTimer] cancelConnectionTimeoutMonitor:peripheral];
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
        // U2F Control Pointへの書き込みを開始
        [self connectedPeripheral:[self connectedPeripheral]
                      writeValues:bleMessages
                forCharacteristic:[self u2fControlPointChar]];
    }

    - (void)connectedPeripheral:(CBPeripheral *)connectedPeripheral
                    writeValues:(NSArray<NSData *> *)valueArray
              forCharacteristic:(CBCharacteristic *)characteristic {
        if (valueArray) {
            // U2F Control Pointへの書込データを保持し、送信済フレーム数をクリア
            [self setBleRequestFrames:valueArray];
            [self setBleRequestFrameNumber:0];
        }
        // U2F Control Pointに、実行するコマンドを書き込み
        NSData *value = [[self bleRequestFrames]
                         objectAtIndex:[self bleRequestFrameNumber]];
        [connectedPeripheral writeValue:value
                      forCharacteristic:characteristic
                                   type:CBCharacteristicWriteWithResponse];
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
        
        // 送信済みフレーム数を設定
        NSLog(@"Sent request [frame=%lu] %@",
              (unsigned long)[self bleRequestFrameNumber],
              [[self bleRequestFrames] objectAtIndex:[self bleRequestFrameNumber]]
              );
        [self setBleRequestFrameNumber:([self bleRequestFrameNumber] + 1)];
        
        if ([self bleRequestFrameNumber] == [[self bleRequestFrames] count]) {
            // 全フレームが送信済であれば、U2F Status経由のレスポンス待ち（タイムアウト監視開始）
            [self centralManagerWillStartResponseTimeout];
            [[self delegate] notifyCentralManagerMessage:MSG_REQUEST_SENT];
        } else {
            // U2F Control Pointへ、後続フレームの書き込みを実行
            [self connectedPeripheral:peripheral writeValues:nil
                    forCharacteristic:characteristic];
        }
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
