//
//  AppBLECommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/14.
//
#import "AppBLECommand.h"
#import "AppBLECommandDefine.h"
#import "AppCommonMessage.h"
#import "FIDODefines.h"
#import "ToolBLEHelper.h"
#import "ToolBLEHelperDefine.h"
#import "ToolCommonFunc.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

@interface AppBLECommand () <ToolBLEHelperDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id              delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) ToolBLEHelper        *toolBLEHelper;
    // 実行コマンドを保持
    @property (nonatomic) Command               command;
    // 送受信データを保持
    @property (nonatomic) NSArray<NSData *>    *bleRequestArray;
    @property (nonatomic) NSData               *bleResponseData;
    @property (nonatomic) uint8_t               bleResponseCmd;
    // 送信フレーム数を保持
    @property (nonatomic) NSUInteger            bleRequestFrameNumber;
    // BLE接続に関する情報を保持
    @property (nonatomic) NSUInteger            bleConnectionRetryCount;
    @property (nonatomic) bool                  bleTransactionStarted;
    @property (nonatomic) NSString             *lastCommandMessage;
    @property (nonatomic) bool                  lastCommandSuccess;
    @property (nonatomic) NSString             *scannedPeripheralName;
    // BLE接続完了済みかどうかを保持（２重デリゲート回避措置）
    @property (nonatomic) bool                  connectedPeripheral;

@end

@implementation AppBLECommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
            [self setToolBLEHelper:[[ToolBLEHelper alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (void)doRequestCommand:(Command)command withCMD:(uint8_t)cmd withData:(NSData *)data {
        // 実行コマンドを退避
        [self setCommand:command];
        // 分割送信のために64バイトごとのコマンド配列を作成する
        [self setBleRequestArray:[self generateCommandArrayFrom:data cmd:cmd]];
        // コマンド配列がブランクの場合は終了
        if ([self commandArrayIsBlank]) {
            return;
        }
        // 接続されている場合（U2F Statusからの受信監視が継続されている場合）
        if ([[self toolBLEHelper] helperIsSubscribingCharacteristic]) {
            // 送信済フレーム数をクリア
            [self setBleRequestFrameNumber:0];
            // U2F Control Pointに、実行するコマンドを書き込み
            NSData *value = [[self bleRequestArray] objectAtIndex:[self bleRequestFrameNumber]];
            [[self toolBLEHelper] helperWillWriteForCharacteristics:value];
        } else {
            // 再試行回数をゼロクリア
            [self setBleConnectionRetryCount:0];
            // メッセージ表示用変数を初期化
            [self setLastCommandMessage:nil];
            [self setLastCommandSuccess:false];
            // BLEデバイス接続処理を開始する
            [self setBleTransactionStarted:false];
            [[self toolBLEHelper] helperWillConnectWithUUID:U2FServiceUUID];
        }
    }

    - (void)commandDidProcess:(bool)result message:(NSString *)message {
        // コマンド配列をブランクに初期化
        [self setBleRequestArray:nil];
        if (message) {
            // コマンド実行結果のメッセージを保持
            [self setLastCommandMessage:message];
        }
        // コマンド実行結果のリザルトを保持
        [self setLastCommandSuccess:result];
        // デバイス接続を切断
        [[self toolBLEHelper] helperWillDisconnect];
    }

#pragma mark - Call back from ToolBLEHelper

    - (void)helperDidScanForPeripheral:(id)peripheralRef scannedPeripheralName:(NSString *)peripheralName withUUID:(NSString *)uuidString withServiceData:(NSData *)serviceData {
        // スキャンされたサービスUUIDを比較し、同じであればペリフェラル接続を試行
        if ([uuidString isEqualToString:@"FFFD"]) {
            // タイムアウトを設定
            NSTimeInterval timeoutSec = U2FSubscrCharTimeoutSec;
            [[self toolBLEHelper] helperWillConnectPeripheral:peripheralRef];
            [self setConnectedPeripheral:false];
            [self setScannedPeripheralName:peripheralName];
            // 接続完了タイマーを開始
            [ToolCommonFunc startTimerWithTarget:self forSelector:@selector(establishConnectionTimedOut) withObject:nil withTimeoutSec:timeoutSec];
        }
    }

    - (bool)deviceIsInPairingMode:(NSData *)serviceDataField {
        // サービスデータフィールドが`0x80`（Device is in pairing mode）になっているかどうか判定
        if (serviceDataField == nil || [serviceDataField length] != 1) {
            return false;
        }
        uint8_t *bytes = (uint8_t *)[serviceDataField bytes];
        return ((bytes[0] & 0x80) == 0x80);
    }

    - (void)helperDidConnectPeripheral {
        // ２重デリゲート回避措置
        if ([self connectedPeripheral] == false) {
            // ログを出力
            [self setConnectedPeripheral:true];
            [[ToolLogFile defaultLogger] info:MSG_U2F_DEVICE_CONNECTED];
            NSString *uuidString = U2FServiceUUID;
            [[self toolBLEHelper] helperWillDiscoverServiceWithUUID:uuidString];
        }
    }

    - (void)helperDidDiscoverService:(id)serviceRef {
        // ログを出力
        [[ToolLogFile defaultLogger] info:MSG_BLE_U2F_SERVICE_FOUND];
        NSArray<NSString *> *characteristicUUIDs = @[U2FControlPointCharUUID, U2FStatusCharUUID];
        [[self toolBLEHelper] helperWillDiscoverCharacteristics:serviceRef withUUIDs:characteristicUUIDs];
    }

    - (void)helperDidDiscoverCharacteristics:(id)serviceRef {
        // データ受信監視を開始
        [[self toolBLEHelper] helperWillSubscribeCharacteristic:serviceRef];
    }

    - (void)helperDidSubscribeCharacteristic {
        // 接続完了タイマーを停止
        [ToolCommonFunc stopTimerWithTarget:self forSelector:@selector(establishConnectionTimedOut) withObject:nil];
        // ログを出力
        [[ToolLogFile defaultLogger] info:MSG_BLE_NOTIFICATION_START];
        // 送信済フレーム数をクリア
        [self setBleRequestFrameNumber:0];
        // U2F Control Pointに、実行するコマンドを書き込み
        NSData *value = [[self bleRequestArray] objectAtIndex:[self bleRequestFrameNumber]];
        [[self toolBLEHelper] helperWillWriteForCharacteristics:value];
        [self setBleTransactionStarted:true];
    }

    - (void)helperDidWriteForCharacteristics {
        // 送信済みフレーム数を設定
        [self setBleRequestFrameNumber:([self bleRequestFrameNumber] + 1)];
        // 後続処理有無の判定
        if ([self bleRequestFrameNumber] == [[self bleRequestArray] count]) {
            // 全フレームが送信済であれば、U2F Status経由のレスポンス待ち（レスポンスタイムアウト監視開始）
            [[self toolBLEHelper] helperWillReadForCharacteristics];
            [[ToolLogFile defaultLogger] info:MSG_REQUEST_SENT];
        } else {
            // U2F Control Pointへ、後続フレームの書き込みを実行
            NSData *value = [[self bleRequestArray] objectAtIndex:[self bleRequestFrameNumber]];
            [[self toolBLEHelper] helperWillWriteForCharacteristics:value];
        }
    }

    - (void)helperDidReadForCharacteristic:(NSData *)responseMessage {
        if ([self isResponseCompleted:responseMessage]) {
            // 後続レスポンスがあれば、タイムアウト監視を再開させ、後続レスポンスを待つ
            [[self toolBLEHelper] helperWillReadForCharacteristics];
        } else {
            // 後続レスポンスがなければ、トランザクション完了と判断
            [[ToolLogFile defaultLogger] info:MSG_RESPONSE_RECEIVED];
            [self setBleTransactionStarted:false];
            // レスポンスを上位クラスに引き渡す
            [[self delegate] didResponseCommand:[self command] response:[self bleResponseData]];
        }
    }

    - (void)helperDidFailConnectionWithError:(NSError *)error reason:(NSUInteger)reason {
        // 接続完了タイマーを停止（接続処理完了前にこのイベントが発生することがあるため）
        [ToolCommonFunc stopTimerWithTarget:self forSelector:@selector(establishConnectionTimedOut) withObject:nil];
        // ログをファイル出力
        NSString *message = [self helperMessageOnFailConnection:reason];
        // 画面上のテキストエリアにもメッセージを表示する
        [self setLastCommandMessage:message];
        // トランザクション完了済とし、接続再試行を回避
        [self setBleTransactionStarted:false];
        // ポップアップ表示させるためのリザルトを保持
        [self setLastCommandSuccess:false];
        // デバイス接続を切断
        [[self toolBLEHelper] helperWillDisconnect];
    }

    - (void)helperDidDisconnectWithError:(NSError *)error peripheral:(id)peripheralRef {
        if (error) {
            // 接続完了タイマーを停止（接続処理完了前にこのイベントが発生することがあるため）
            [ToolCommonFunc stopTimerWithTarget:self forSelector:@selector(establishConnectionTimedOut) withObject:nil];
            // エラーをログ出力した後、接続を切断
            [[ToolLogFile defaultLogger] errorWithFormat:@"BLE disconnected with message: %@", [error description]];
            [[self toolBLEHelper] helperWillDisconnectForce:peripheralRef];
            return;
        }
        // 上位クラスに完了通知を行う
        [[self delegate] didCompleteCommand:[self command] success:[self lastCommandSuccess] errorMessage:[self lastCommandMessage]];
    }

    - (NSString *)nameOfScannedPeripheral {
        // スキャンが成功したペリフェラルの名前を戻す
        return [self scannedPeripheralName];
    }

    - (void)establishConnectionTimedOut {
        // 接続完了タイムアウト発生時の処理
        [self setLastCommandMessage:MSG_U2F_DEVICE_ESTABLISH_CONN_TIMEOUT];
        [self setLastCommandSuccess:false];
        // デバイス接続を切断
        [[self toolBLEHelper] helperWillDisconnect];
    }

#pragma mark - Function for sending data frames

    - (NSArray<NSData *> *)generateCommandArrayFrom:(NSData *)dataForCommand cmd:(uint8_t)cmd {
        unsigned char initHeader[] = {cmd, 0x00, 0x00};
        unsigned char contHeader[] = {0x00};

        NSUInteger dataForCommandLength = [dataForCommand length];
        NSUInteger start    = 0;
        char       sequence = 0;
        uint16_t   dump_data_len;

        NSMutableArray<NSData *> *array = [[NSMutableArray alloc] init];
        while (start < dataForCommandLength) {
            NSMutableData *dataRequest = [NSMutableData alloc];
            NSData *dataHeader;
            
            NSUInteger strlen = dataForCommandLength - start;
            if (start == 0) {
                // 最大61バイト分取得する
                if (strlen > 61) {
                    strlen = 61;
                }
                // BLEヘッダーにリクエストデータ長を設定する
                initHeader[1] = dataForCommandLength / 256;
                initHeader[2] = dataForCommandLength % 256;
                dataHeader = [[NSData alloc] initWithBytes:initHeader length:sizeof(initHeader)];
                // ログ出力
                [[ToolLogFile defaultLogger]
                 debugWithFormat:@"BLE Sent INIT frame: data size=%d length=%d", dataForCommandLength, strlen];
                dump_data_len = strlen + sizeof(initHeader);
                
            } else {
                // 最大63バイト分取得する
                if (strlen > 63) {
                    strlen = 63;
                }
                // BLEヘッダーにシーケンス番号を設定する
                contHeader[0] = sequence;
                dataHeader = [[NSData alloc] initWithBytes:contHeader length:sizeof(contHeader)];
                // ログ出力
                [[ToolLogFile defaultLogger]
                 debugWithFormat:@"BLE Sent CONT frame: seq=%d length=%d", sequence++, strlen];
                dump_data_len = strlen + sizeof(contHeader);
            }

            // スタート位置からstrlen文字分切り出して、ヘッダーに連結
            [dataRequest appendData:dataHeader];
            [dataRequest appendData:[dataForCommand subdataWithRange:NSMakeRange(start, strlen)]];
            [array addObject:dataRequest];
            // フレーム内容をログ出力
            [[ToolLogFile defaultLogger] hexdump:dataRequest];
            // スタート位置を更新
            start += strlen;
        }
        
        return array;
    }

    - (bool)commandArrayIsBlank {
        if ([self bleRequestArray]) {
            if ([[self bleRequestArray] count]) {
                return false;
            }
        }
        return true;
    }

#pragma mark - Function for receiving data frames

    - (bool)isResponseCompleted:(NSData *)responseData {
        // 受信データおよび長さを保持
        static NSUInteger     totalLength;
        static NSMutableData *receivedData;
        
        // 後続データの存在有無をチェック
        NSData *dataBLEHeader = [responseData subdataWithRange:NSMakeRange(0, 3)];
        unsigned char *bytesBLEHeader = (unsigned char *)[dataBLEHeader bytes];
        if (bytesBLEHeader[0] & 0x80) {
            // INITフレームの場合は、CMDを退避しておく
            [self setBleResponseCmd:bytesBLEHeader[0]];
        }
        if ([self isBLEKeepaliveByte:bytesBLEHeader[0]]) {
            // キープアライブの場合は引き続き次のレスポンスを待つ
            receivedData = nil;
            
        } else if ([self isBLECommandByte:bytesBLEHeader[0]]) {
            // ヘッダーから全受信データ長を取得
            totalLength  = bytesBLEHeader[1] * 256 + bytesBLEHeader[2];
            // 4バイト目から後ろを切り出して連結
            NSData *tmp  = [responseData subdataWithRange:NSMakeRange(3, [responseData length] - 3)];
            receivedData = [[NSMutableData alloc] initWithData:tmp];
            // ログ出力
            [[ToolLogFile defaultLogger]
             debugWithFormat:@"BLE Recv INIT frame: data size=%d length=%d", totalLength, [tmp length]];
            [[ToolLogFile defaultLogger] hexdump:responseData];

        } else {
            // 2バイト目から後ろを切り出して連結
            NSData *tmp  = [responseData subdataWithRange:NSMakeRange(1, [responseData length] - 1)];
            [receivedData appendData:tmp];
            // ログ出力
            uint8_t *b = (uint8_t *)[responseData bytes];
            [[ToolLogFile defaultLogger]
             debugWithFormat:@"BLE Recv CONT frame: seq=%d length=%d", b[0], [tmp length]];
            [[ToolLogFile defaultLogger] hexdump:responseData];
        }
        
        if (receivedData && ([receivedData length] == totalLength)) {
            // 全受信データを保持
            [self setBleResponseData:[[NSData alloc] initWithData:receivedData]];
            receivedData = nil;
            // 後続レスポンスがない
            return false;
            
        } else {
            // 後続レスポンスがある
            [self setBleResponseData:nil];
            return true;
        }
    }

    - (bool)isBLEKeepaliveByte:(uint8_t)commandByte {
        // キープアライブの場合は true
        return (commandByte == 0x82);
    }

    - (bool)isBLECommandByte:(uint8_t)commandByte {
        // BLEコマンドバイトの場合は true
        switch (commandByte) {
            case 0x81:
            case 0x83:
            case HID_CMD_UNKNOWN_ERROR:
                return true;
            default:
                return false;
        }
    }

#pragma mark - Private functions

    - (NSString *)helperMessageOnFailConnection:(BLEErrorReason)reason {
        // BLE処理時のエラーコードを、適切なメッセージに変更する
        switch (reason) {
            case BLE_ERR_BLUETOOTH_OFF:
                return MSG_BLE_PARING_ERR_BT_OFF;
            case BLE_ERR_DEVICE_CONNECT_FAILED:
                return MSG_U2F_DEVICE_CONNECT_FAILED;
            case BLE_ERR_DEVICE_SCAN_TIMEOUT:
                if ([self command] == COMMAND_PAIRING) {
                    return MSG_BLE_PARING_ERR_TIMED_OUT;
                } else {
                    return MSG_U2F_DEVICE_SCAN_TIMEOUT;
                }
            case BLE_ERR_SERVICE_NOT_DISCOVERED:
                return MSG_BLE_SERVICE_NOT_DISCOVERED;
            case BLE_ERR_SERVICE_NOT_FOUND:
                return MSG_BLE_U2F_SERVICE_NOT_FOUND;
            case BLE_ERR_CHARACT_NOT_DISCOVERED:
                return MSG_BLE_CHARACT_NOT_DISCOVERED;
            case BLE_ERR_CHARACT_NOT_EXIST:
                return MSG_BLE_CHARACT_NOT_EXIST;
            case BLE_ERR_NOTIFICATION_FAILED:
                if ([self command] == COMMAND_PAIRING) {
                    return MSG_BLE_PARING_ERR_PAIR_MODE;
                } else {
                    return MSG_BLE_NOTIFICATION_FAILED;
                }
            case BLE_ERR_NOTIFICATION_STOP:
                return MSG_BLE_NOTIFICATION_STOP;
            case BLE_ERR_REQUEST_SEND_FAILED:
                return MSG_REQUEST_SEND_FAILED;
            case BLE_ERR_RESPONSE_RECEIVE_FAILED:
                return MSG_RESPONSE_RECEIVE_FAILED;
            case BLE_ERR_REQUEST_TIMEOUT:
                return MSG_REQUEST_TIMEOUT;
            default:
                if ([self command] == COMMAND_PAIRING) {
                    return MSG_BLE_PARING_ERR_UNKNOWN;
                } else {
                    return MSG_OCCUR_BLECONN_ERROR;
                }
        }
    }

@end
