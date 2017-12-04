#import "ToolBLECentral.h"
#import "ToolCommand.h"

static const NSTimeInterval kScanningTimeout   = 10.0;
static const NSTimeInterval kConnectingTimeout = 10.0;
static const NSTimeInterval kRequestTimeout    = 20.0;

@interface ToolBLECentral () <CBCentralManagerDelegate, CBPeripheralDelegate>

    @property(nonatomic) CBCentralManager *manager;
    @property(nonatomic) CBPeripheral     *connectedPeripheral;
    @property(nonatomic) CBService        *connectedService;

    @property(nonatomic) CBCharacteristic *u2fControlPointChar;
    @property(nonatomic) CBCharacteristic *u2fStatusChar;

    @property(nonatomic) ToolCommand      *toolCommand;

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
        }
        return self;
    }

    - (void)centralManagerDidUpdateState:(CBCentralManager *)central {
        NSLog(@"centralManagerDidUpdateState: %ld", central.state);
    }

#pragma mark - Notify to appDelegate

    - (void)notifyMessageToAppDelegate:(NSString *)message {
        if (message) {
            NSLog(@"%@", message);
        }
        [self.delegate notifyMessage:message];
    }

    - (void)notifyFailMessageToAppDelegate:(NSString *)message {
        [self notifyMessageToAppDelegate:message];
        [self.delegate notifyFailWithMessage:message];
    }

    - (void)notifyErrorMessageToAppDelegate:(NSString *)message error:(NSError *)err {
        if (err) {
            NSLog(@"%@ %@", message, [err description]);
        }
        [self.delegate notifyMessage:message];
        [self.delegate notifyFailWithMessage:message];
    }

    - (void)notifySuccessToAppDelegate {
        [self.delegate notifySuccess];
    }

#pragma mark - Entry for process

    - (void)doCommand:(ToolCommand *)toolCommand {
        NSAssert(self.serviceUUIDs.count > 0, @"Need to specify services");
        NSAssert(self.characteristicUUIDs.count > 0, @"Need to specify characteristics UUID");

        if (self.manager.state != CBCentralManagerStatePoweredOn) {
            [self notifyFailMessageToAppDelegate:@"BLEが無効化されています。BLEを有効にしてください。"];
            return;
        }
        
        // 実行するコマンドを編集
        self.toolCommand = toolCommand;
        if ([self.toolCommand createCommandArrayFor:[self.toolCommand command] fromData:nil] == false) {
            [self notifyFailMessageToAppDelegate:[self.toolCommand lastOccuredErrorMessage]];
            return;
        }

        // FIDO BLE U2Fサービスを有するペリフェラルのスキャン開始
        [self scanForPeripherals];
    }

#pragma mark - Scan for peripherals

    - (void)scanForPeripherals {
        if (self.manager.state != CBCentralManagerStatePoweredOn) {
            return;
        }
        [self notifyMessageToAppDelegate:@"FIDO U2Fデバイスのスキャンを開始します"];

        // スキャン設定
        [self startScanningTimeoutMonitor];
        NSDictionary *scanningOptions = @{CBCentralManagerScanOptionAllowDuplicatesKey : @NO};

        // FIDO BLE U2Fサービスを持つペリフェラルをスキャン
        self.connectedPeripheral = nil;
        [self.manager scanForPeripheralsWithServices:nil options:scanningOptions];
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
            [self notifyMessageToAppDelegate:@"FIDO U2Fデバイスのスキャンを完了しました"];
            [self cancelScanForPeripherals];
            [self cancelScanningTimeoutMonitor];
            [self connectPeripheral:peripheral];
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
        [self cancelScanForPeripherals];
        [self notifyFailMessageToAppDelegate:@"FIDO U2Fデバイスのスキャンがタイムアウトしました。"];
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
        [self.manager cancelPeripheralConnection:peripheral];
        [self notifyFailMessageToAppDelegate:@"FIDO U2Fデバイスの接続要求がタイムアウトしました。"];
    }

#pragma mark - Connect peripheral

    - (void)connectPeripheral:(CBPeripheral *)peripheral {
        // ペリフェラルに接続
        [self.manager connectPeripheral:peripheral options:nil];
        [self startConnectionTimeoutMonitor:peripheral];
    }

    - (void)centralManager:(CBCentralManager *)central
            didConnectPeripheral:(CBPeripheral *)peripheral {
        // すでに接続されている状態の場合は終了
        if (self.connectedPeripheral) {
            return;
        }
        // 接続されたペリフェラルの参照を保持
        self.connectedPeripheral = peripheral;
        [self notifyMessageToAppDelegate:@"FIDO U2Fデバイスに接続しました。"];

        // FIDO BLE U2Fサービスのディスカバーを開始
        [self cancelConnectionTimeoutMonitor:peripheral];
        [self discoverServices:peripheral];
    }

    - (void)centralManager:(CBCentralManager *)central
didFailToConnectPeripheral:(CBPeripheral *)peripheral
                     error:(NSError *)error {
        [self cancelConnectionTimeoutMonitor:peripheral];
        [self notifyErrorMessageToAppDelegate:@"FIDO U2Fデバイスの接続に失敗しました。" error:error];
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
        [self notifyMessageToAppDelegate:@"FIDO U2Fデバイスの接続を切断しました。"];

        // 終了メッセージを表示
        if ([self.toolCommand lastOccuredErrorMessage]) {
            [self notifyFailMessageToAppDelegate:[self.toolCommand lastOccuredErrorMessage]];

        } else if ([self.toolCommand commandSuccess]) {
            [self notifySuccessToAppDelegate];
        }
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
            // BLEサービスディスカバーに失敗時は、画面にエラーメッセージを表示
            [self notifyErrorMessageToAppDelegate:@"BLEサービスが見つかりません。" error:error];
            return;
        }

        // FIDO BLE U2Fサービスを判定し、その参照を保持
        self.connectedService = nil;
        for (CBService *service in peripheral.services) {
            if ([self.serviceUUIDs containsObject:service.UUID]) {
                self.connectedService = service;
                [self notifyMessageToAppDelegate:@"FIDO BLE U2Fサービスが見つかりました。"];
                break;
            }
        }

        if (!self.connectedService) {
            // FIDO BLE U2Fサービスがない場合は、画面にエラーメッセージを表示
            [self notifyFailMessageToAppDelegate:@"FIDO BLE U2Fサービスが見つかりません。"];
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
            // キャラクタリスティックのディスカバーエラー発生時は、以降の処理を行わない
            [self notifyErrorMessageToAppDelegate:@"FIDO BLE U2Fサービスと通信できません。" error:error];
            return;
        }

        // U2F Statusキャラクタリスティックに対する監視を開始
        [self subscribeCharacteristic:service];
    }

#pragma mark - Subscribe characteristic

    - (void)subscribeCharacteristic:(CBService *)service {
        // サービスの参照を保持
        self.connectedService = service;

        // サービスにキャラクタリスティックがない場合は終了
        if (service.characteristics.count < 1) {
            [self disconnect];
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

    - (void)unsubscribeCharacteristic:(CBService *)service {
         // U2F Statusキャラクタリスティックに対する監視を停止
         if (self.u2fStatusChar) {
             [self.connectedPeripheral setNotifyValue:NO forCharacteristic:self.u2fStatusChar];
         }
    }

    - (void)peripheral:(CBPeripheral *)peripheral
        didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic
        error:(NSError *)error {
        if (error) {
            // エラーメッセージを画面表示し切断処理実行
            [self notifyErrorMessageToAppDelegate:@"FIDO BLE U2Fサービスからデータを受信できません。" error:error];
            [self disconnect];
            return;
        }

        if (characteristic.isNotifying) {
            // コマンドの実行を開始
            [self notifyMessageToAppDelegate:@"受信データの監視を開始します。"];
            [self executeCommandArray];
        } else {
            // 切断処理
            [self notifyMessageToAppDelegate:@"受信データの監視を停止します。"];
            [self disconnect];
        }
    }

#pragma mark - Do main process

    - (void)executeCommandArray {
        // U2F Control Pointに、実行するコマンドを書き込み
        for (NSData *data in self.toolCommand.commandArray) {
            [NSThread sleepForTimeInterval:0.25];
            [self.connectedPeripheral writeValue:data
                               forCharacteristic:self.u2fControlPointChar
                                            type:CBCharacteristicWriteWithResponse];
            NSLog(@"Sent request %@", data);
        }
        [self notifyMessageToAppDelegate:@"リクエストを送信しました。"];
    }

#pragma mark - Request timeout monitor

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
        [self disconnect];
        [self notifyFailMessageToAppDelegate:@"リクエストがタイムアウトしました。"];
    }

#pragma mark - Write value for characteristics

    - (void)peripheral:(CBPeripheral *)peripheral
            didWriteValueForCharacteristic:(CBCharacteristic *)characteristic
            error:(NSError *)error {
        if (error) {
            // U2F Control Point書込エラー発生時はメッセージを画面表示
            [self notifyErrorMessageToAppDelegate:@"リクエスト送信が失敗しました。" error:error];
            return;
        }

        // U2F Status経由のレスポンス待ち（タイムアウト監視開始）
        [self startRequestTimeout:self.u2fStatusChar];
    }

    - (void)peripheral:(CBPeripheral *)peripheral
            didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic
            error:(NSError *)error {
        // 受信データおよび長さを保持
        static NSUInteger     totalLength;
        static NSMutableData *receivedData;

        // タイムアウト監視を停止
        [self cancelRequestTimeoutMonitor:self.u2fStatusChar];

        // U2F Status監視エラー発生時はメッセージを画面表示
        if (error) {
            [self notifyErrorMessageToAppDelegate:@"レスポンスを受信できませんでした。" error:error];
            return;
        }

        // 受信データ
        NSData *responseData = characteristic.value;
        
        // 後続データの存在有無をチェック
        NSData *dataBLEHeader = [responseData subdataWithRange:NSMakeRange(0, 3)];
        unsigned char *bytesBLEHeader = (unsigned char *)[dataBLEHeader bytes];
        if (bytesBLEHeader[0] == 0x82) {
            // キープアライブの場合は引き続き次のレスポンスを待つ
            receivedData = nil;

        } else if (bytesBLEHeader[0] == 0x83) {
            // ヘッダーから全受信データ長を取得
            totalLength  = bytesBLEHeader[1] * 256 + bytesBLEHeader[2];
            // 4バイト目から後ろを切り出して連結
            NSData *tmp  = [responseData subdataWithRange:NSMakeRange(3, [responseData length] - 3)];
            receivedData = [[NSMutableData alloc] initWithData:tmp];
        } else {
            // 2バイト目から後ろを切り出して連結
            NSData *tmp  = [responseData subdataWithRange:NSMakeRange(1, [responseData length] - 1)];
            [receivedData appendData:tmp];
        }
        NSLog(@"Received response %@", responseData);

        if (receivedData && ([receivedData length] == totalLength)) {
            // 全データを受信したら後続の処理を行う
            [self notifyMessageToAppDelegate:@"レスポンスを受信しました。"];
            [self doAfterResponseFromCharacteristicWith:receivedData];
            receivedData = nil;

        } else {
            // 後続のレスポンス待ち（タイムアウト監視開始）
            [self startRequestTimeout:self.u2fStatusChar];
        }
    }

#pragma mark - Process after response from peripheral

    - (void)doAfterResponseFromCharacteristicWith:(NSData *)responseData {
        // 後続処理がある場合は主処理に戻る
        if ([self.toolCommand doAfterResponseFor:[self.toolCommand command] withData:responseData]) {
            [self executeCommandArray];
            return;
        }

        // 後続処理がない場合は切断処理を実行
        [self disconnect];
    }

#pragma mark - Disconnect from peripheral

    - (void)disconnect {
        // ペリフェラル接続を切断
        if (self.connectedPeripheral) {
            [self cancelScanForPeripherals];
            [self.manager cancelPeripheralConnection:self.connectedPeripheral];
        }
    }

@end
