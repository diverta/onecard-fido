#import "ToolBLECentral.h"
#import "ToolCommonMessage.h"
#import "ToolTimer.h"

#define U2FServiceUUID          @"0000FFFD-0000-1000-8000-00805F9B34FB"
#define U2FControlPointCharUUID @"F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB"
#define U2FStatusCharUUID       @"F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB"
#define PairingModeSignCharUUID @"98439EE6-776B-401C-880C-682FBDDD8E32"

@interface ToolBLECentral () <CBCentralManagerDelegate, CBPeripheralDelegate, ToolTimerDelegate>

    @property(nonatomic) CBCentralManager *manager;
    @property(nonatomic) CBPeripheral     *connectedPeripheral;
    @property(nonatomic) CBService        *connectedService;

    @property(nonatomic) CBCharacteristic *u2fControlPointChar;
    @property(nonatomic) CBCharacteristic *u2fStatusChar;
    @property(nonatomic) CBCharacteristic *pairingModeSignChar;

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

        if (self.manager.state != CBCentralManagerStatePoweredOn) {
            // BLEが無効化されている旨をAppDelegateに通知
            [[self delegate] centralManagerDidFailConnectionWith:MSG_INVALID_BLE_PERIPHERAL error:nil];
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
        [[self delegate] notifyCentralManagerMessage:MSG_U2F_DEVICE_SCAN_STOPPED];
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
            break;
        }
    }

    - (void)scanningDidTimeout {
        // スキャンを停止
        [self cancelScanForPeripherals];
        // スキャンタイムアウトの旨をAppDelegateに通知
        [[self delegate] centralManagerDidFailConnectionWith:MSG_U2F_DEVICE_SCAN_TIMEOUT error:nil];
    }

    - (void)connectionDidTimeout {
        // 接続タイムアウトの旨をAppDelegateに通知
        [[self delegate] centralManagerDidFailConnectionWith:MSG_U2F_DEVICE_CONNREQ_TIMEOUT error:nil];
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
        [[self delegate] centralManagerDidFailConnectionWith:MSG_U2F_DEVICE_CONNECT_FAILED error:error];
    }

    - (void)centralManager:(CBCentralManager *)central
   didDisconnectPeripheral:(CBPeripheral *)peripheral
                     error:(NSError *)error {
        // レスポンスタイムアウト監視を停止
        if ([self u2fStatusChar]) {
            [[self toolTimer] cancelResponseTimeoutMonitor:[self u2fStatusChar]];
        }
        if ([self pairingModeSignChar]) {
            [[self toolTimer] cancelResponseTimeoutMonitor:[self pairingModeSignChar]];
        }
        // ペリフェラル、サービス、キャラクタリスティックの参照を解除
        [self setConnectedPeripheral:nil];
        [self setConnectedService:nil];
        [self setU2fControlPointChar:nil];
        [self setU2fStatusChar:nil];
        [self setPairingModeSignChar:nil];
        // 切断完了
        [[self delegate] centralManagerDidDisconnectWith:MSG_U2F_DEVICE_DISCONNECTED error:error];
    }

#pragma mark - Discover services

    - (void)discoverServices:(CBPeripheral *)peripheral {
        // FIDO BLE U2Fサービスのディスカバーを開始
        [peripheral setDelegate:self];
        [peripheral discoverServices:nil];
        // サービス・ディスカバーのタイムアウト監視を開始
        [[self toolTimer] startDiscoverServicesTimeoutMonitor:peripheral];
    }

    - (void)discoverServicesDidTimeout {
        // サービスディスカバータイムアウトの旨をAppDelegateに通知
        [[self delegate] centralManagerDidFailConnectionWith:MSG_DISCOVER_U2F_SERVICES_TIMEOUT error:nil];
    }

    - (void)peripheral:(CBPeripheral *)peripheral
   didDiscoverServices:(NSError *)error {
        // サービス・ディスカバーのタイムアウト監視を停止
        [[self toolTimer] cancelDiscoverServicesTimeoutMonitor:peripheral];
        
        if (error) {
            // BLEサービスディスカバーに失敗の旨をAppDelegateに通知
            [[self delegate] centralManagerDidFailConnectionWith:MSG_BLE_SERVICE_NOT_DISCOVERED error:error];
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
            [[self delegate] centralManagerDidFailConnectionWith:MSG_BLE_U2F_SERVICE_NOT_FOUND error:nil];
            return;
        }

        // キャラクタリスティックのディスカバーを実行
        [self discoverServiceCharacteristics:self.connectedService];
    }

#pragma mark - Discover characteristics

    - (void)discoverServiceCharacteristics:(CBService *)service {
        // サービス内のキャラクタリスティックをディスカバー
        [self.connectedPeripheral discoverCharacteristics:nil
                                               forService:service];
        // キャラクタリスティック・ディスカバーのタイムアウト監視を開始
        [[self toolTimer] startDiscoverCharacteristicsTimeoutMonitor:service];
    }

    - (void)discoverCharacteristicsDidTimeout {
        // キャラクタリスティック・ディスカバータイムアウトの旨をAppDelegateに通知
        [[self delegate] centralManagerDidFailConnectionWith:MSG_DISCOVER_U2F_CHARAS_TIMEOUT error:nil];
    }

    - (void)peripheral:(CBPeripheral *)peripheral
            didDiscoverCharacteristicsForService:(CBService *)service
            error:(NSError *)error {
        // キャラクタリスティック・ディスカバーのタイムアウト監視を停止
        [[self toolTimer] cancelDiscoverCharacteristicsTimeoutMonitor:service];
        
        if (error) {
            // キャラクタリスティックのディスカバーエラー発生の旨をAppDelegateに通知
            [[self delegate] centralManagerDidFailConnectionWith:MSG_BLE_CHARACT_NOT_DISCOVERED error:error];
            return;
        }

        // サービスの参照を保持
        [self setConnectedService:service];
        if ([[service characteristics] count] < 1) {
            // サービスにキャラクタリスティックがない旨をAppDelegateに通知
            [[self delegate] centralManagerDidFailConnectionWith:MSG_BLE_CHARACT_NOT_EXIST error:nil];
            return;
        }
        // キャラクタリスティックの参照を保持
        [self setU2fControlPointChar:nil];
        [self setU2fStatusChar:nil];
        [self setPairingModeSignChar:nil];
        for (CBCharacteristic *characteristic in service.characteristics) {
            NSString *uuidString = [[characteristic UUID] UUIDString];
            if ([uuidString isEqualToString:U2FControlPointCharUUID]) {
                NSLog(@"U2F control point characteristic discovered.");
                [self setU2fControlPointChar:characteristic];
                
            } else if ([uuidString isEqualToString:U2FStatusCharUUID]) {
                NSLog(@"U2F status characteristic discovered.");
                [self setU2fStatusChar:characteristic];
                
            } else if ([uuidString isEqualToString:PairingModeSignCharUUID]) {
                NSLog(@"Pairing mode sign characteristic discovered.");
                [self setPairingModeSignChar:characteristic];
            }
        }
        // 一連の接続処理が完了したことをAppDelegateに通知
        [[self delegate] centralManagerDidConnect];
    }

#pragma mark - Subscribe characteristic

    - (void)centralManagerWillStartSubscribe {
        // U2F statusに対する監視を開始する
        if ([self u2fStatusChar]) {
            if ([[self u2fStatusChar] isNotifying]) {
                [[self delegate] centralManagerDidStartSubscribe];
            } else {
                [[self connectedPeripheral] setNotifyValue:YES forCharacteristic:[self u2fStatusChar]];
                [[self toolTimer] startSubscribeCharacteristicTimeoutMonitor:[self u2fStatusChar]];
            }
        }
    }

    - (void)subscribeCharacteristicDidTimeout {
        // 監視ステータス更新タイムアウトの旨をAppDelegateに通知
        [[self delegate] centralManagerDidFailConnectionWith:MSG_SUBSCRIBE_U2F_STATUS_TIMEOUT error:nil];
    }

    - (void)peripheral:(CBPeripheral *)peripheral
        didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic
        error:(NSError *)error {
        // 監視ステータス更新のタイムアウト監視を停止
        [[self toolTimer] cancelSubscribeCharacteristicTimeoutMonitor:characteristic];
        
        if (error) {
            // U2F Status監視開始エラー発生の旨をAppDelegateに通知
            [[self delegate] centralManagerDidFailConnectionWith:MSG_BLE_NOTIFICATION_FAILED error:error];
            return;
        }

        if (characteristic.isNotifying) {
            // U2F status監視開始処理が完了したことをAppDelegateに通知
            [[self delegate] notifyCentralManagerMessage:MSG_BLE_NOTIFICATION_START];
            [[self delegate] centralManagerDidStartSubscribe];
        } else {
            // 監視が停止している旨をAppDelegateに通知
            [[self delegate] centralManagerDidFailConnectionWith:MSG_BLE_NOTIFICATION_STOP error:nil];
        }
    }

#pragma mark - Do main process

    - (void)centralManagerWillReadParingModeSign {
        // ペアリングモード標識の値を読み込む（レスポンスタイムアウト監視開始）
        if ([self centralManagerHasParingModeSign]) {
            [[self connectedPeripheral] readValueForCharacteristic:[self pairingModeSignChar]];
            [[self toolTimer] startResponseTimeoutMonitor:[self pairingModeSignChar]];
        }
    }

    - (bool)centralManagerHasParingModeSign {
        return ([self pairingModeSignChar] != nil);
    }

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

#pragma mark - Response timeout monitor

    - (void)centralManagerWillStartResponseTimeout {
        // U2F Status経由のレスポンス待ち（レスポンスタイムアウト監視開始）
        [[self toolTimer] startResponseTimeoutMonitor:[self u2fStatusChar]];
    }

    - (void)responseDidTimeout {
        // レスポンスタイムアウト発生の旨をAppDelegateに通知
        [[self delegate] centralManagerDidFailConnectionWith:MSG_REQUEST_TIMEOUT error:nil];
    }

#pragma mark - Write value for characteristics

    - (void)peripheral:(CBPeripheral *)peripheral
            didWriteValueForCharacteristic:(CBCharacteristic *)characteristic
            error:(NSError *)error {
        if (error) {
            // U2F Control Point書込エラー発生の旨をAppDelegateに通知
            [[self delegate] centralManagerDidFailConnectionWith:MSG_REQUEST_SEND_FAILED error:error];
            return;
        }
        
        // 送信済みフレーム数を設定
        NSLog(@"Sent request [frame=%lu] %@",
              (unsigned long)[self bleRequestFrameNumber],
              [[self bleRequestFrames] objectAtIndex:[self bleRequestFrameNumber]]
              );
        [self setBleRequestFrameNumber:([self bleRequestFrameNumber] + 1)];
        
        if ([self bleRequestFrameNumber] == [[self bleRequestFrames] count]) {
            // 全フレームが送信済であれば、U2F Status経由のレスポンス待ち（レスポンスタイムアウト監視開始）
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
        // レスポンスタイムアウト監視を停止
        [[self toolTimer] cancelResponseTimeoutMonitor:characteristic];

        if (error) {
            // U2F Status取得エラー発生の旨をAppDelegateに通知
            [[self delegate] centralManagerDidFailConnectionWith:MSG_RESPONSE_RECEIVE_FAILED error:error];
            return;
        }

        // 受信データをAppDelegateへ転送
        [self.delegate centralManagerDidReceive:[characteristic value]];
    }

#pragma mark - Disconnect from peripheral

    - (void)centralManagerWillDisconnect {
        if (self.connectedPeripheral) {
            // ペリフェラル接続を切断
            [self.manager cancelPeripheralConnection:self.connectedPeripheral];
        } else {
            [[self delegate] centralManagerDidDisconnectWith:nil error:nil];
        }
    }

@end
