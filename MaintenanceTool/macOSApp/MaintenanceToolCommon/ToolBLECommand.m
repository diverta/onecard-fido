//
//  ToolBLECommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2017/11/20.
//
#import <Foundation/Foundation.h>
#import "FIDODefines.h"
#import "ToolBLEHelper.h"
#import "ToolBLECommand.h"
#import "ToolCommon.h"
#import "ToolCommonMessage.h"
#import "ToolCTAP2HealthCheckCommand.h"
#import "ToolU2FHealthCheckCommand.h"

@interface ToolBLECommand () <ToolBLEHelperDelegate>
    // コマンドを保持
    @property (nonatomic) Command   command;
    // 送信PINGデータを保持
    @property (nonatomic) NSData   *pingData;
    // BLE接続に関する情報を保持
    @property (nonatomic) ToolBLEHelper     *toolBLECentral;
    @property (nonatomic) NSUInteger         bleConnectionRetryCount;
    @property (nonatomic) bool               bleTransactionStarted;
    @property (nonatomic) NSString          *lastCommandMessage;
    @property (nonatomic) bool               lastCommandSuccess;
    // 送受信データを保持
    @property (nonatomic) NSArray<NSData *> *bleRequestArray;
    @property (nonatomic) NSData            *bleResponseData;
    @property (nonatomic) uint8_t            bleResponseCmd;
    // 処理クラス
    @property (nonatomic) ToolCTAP2HealthCheckCommand *toolCTAP2HealthCheckCommand;
    @property (nonatomic) ToolU2FHealthCheckCommand   *toolU2FHealthCheckCommand;
@end

@implementation ToolBLECommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolBLECommandDelegate>)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
            [self setToolBLECentral:[[ToolBLEHelper alloc] initWithDelegate:self]];
            [self setToolCTAP2HealthCheckCommand:[[ToolCTAP2HealthCheckCommand alloc] init]];
            [[self toolCTAP2HealthCheckCommand] setTransportParam:TRANSPORT_BLE
                                                   toolBLECommand:self
                                                   toolHIDCommand:nil];
            [self setToolU2FHealthCheckCommand:[[ToolU2FHealthCheckCommand alloc] init]];
            [[self toolU2FHealthCheckCommand] setTransportParam:TRANSPORT_BLE
                                                 toolBLECommand:self
                                                 toolHIDCommand:nil];
        }
        return self;
    }

#pragma mark - Private methods

    - (void)doRequestCommandPairing {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        NSLog(@"Pairing start");
        
        // 書き込むコマンド（APDU）を編集
        unsigned char arr[] = {0x00, 0x45, 0x00, 0x00};
        NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
        [self doBLECommandRequestFrom:commandData cmd:0x83];
    }

    - (void)doResponseCommandPairing:(NSData *)message {
        // ステータスコードを確認し、画面に制御を戻す
        [self commandDidProcess:[[self toolU2FHealthCheckCommand] checkStatusWordOfResponse:message]
                        message:nil];
    }

    - (void)doRequestCommandPing {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        NSLog(@"BLE ping start");
        // 100バイトのランダムなPINGデータを生成
        [self setPingData:[ToolCommon generateRandomBytesDataOf:100]];
        // 分割送信のために64バイトごとのコマンド配列を作成する
        [self doBLECommandRequestFrom:[self pingData] cmd:0x81];
    }

    - (void)doResponseCommandPing {
        // PINGレスポンスの内容をチェックし、画面に制御を戻す
        bool result = [[self bleResponseData] isEqualToData:[self pingData]];
        [self commandDidProcess:result message:@"BLE ping end"];
    }

    - (void)doBLECommandRequestFrom:(NSData *)dataForCommand cmd:(uint8_t)cmd {
        // 分割送信のために64バイトごとのコマンド配列を作成する
        [self setBleRequestArray:[self generateCommandArrayFrom:dataForCommand cmd:cmd]];
        // コマンド配列がブランクの場合は終了
        if ([self commandArrayIsBlank]) {
            return;
        }
        // 再試行回数をゼロクリアし、BLEデバイス接続処理に移る
        [self setBleConnectionRetryCount:0];
        [self startBleConnection];
    }

    - (NSArray<NSData *> *)generateCommandArrayFrom:(NSData *)dataForCommand cmd:(uint8_t)cmd {
        unsigned char initHeader[] = {cmd, 0x00, 0x00};
        unsigned char contHeader[] = {0x00};

        NSUInteger dataForCommandLength = [dataForCommand length];
        NSUInteger start    = 0;
        char       sequence = 0;

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

            } else {
                // 最大63バイト分取得する
                if (strlen > 63) {
                    strlen = 63;
                }
                // BLEヘッダーにシーケンス番号を設定する
                contHeader[0] = sequence++;
                dataHeader = [[NSData alloc] initWithBytes:contHeader length:sizeof(contHeader)];
            }

            // スタート位置からstrlen文字分切り出して、ヘッダーに連結
            [dataRequest appendData:dataHeader];
            [dataRequest appendData:[dataForCommand subdataWithRange:NSMakeRange(start, strlen)]];
            [array addObject:dataRequest];
            
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

    - (void)doCtap2HealthCheck {
        // コマンド開始メッセージを画面表示
        if ([self command] == COMMAND_TEST_MAKE_CREDENTIAL) {
            [self displayStartMessage];
        }
        // まず最初にGetKeyAgreementサブコマンドを実行
        [[self toolCTAP2HealthCheckCommand] doCTAP2Request:[self command]];
    }

    - (void)doU2FHealthCheck {
        // コマンド開始メッセージを画面表示
        if ([self command] == COMMAND_TEST_REGISTER) {
            [self displayStartMessage];
        }
        // まず最初にU2F Registerコマンドを実行
        [[self toolU2FHealthCheckCommand] doU2FRequest:[self command]];
    }

#pragma mark - Public methods

    - (void)bleCommandWillProcess:(Command)command {
        // コマンドに応じ、以下の処理に分岐
        [self setCommand:command];
        switch (command) {
            case COMMAND_TEST_REGISTER:
                // U2Fコマンドを生成して実行
                [self doU2FHealthCheck];
                break;
            case COMMAND_PAIRING:
                [self doRequestCommandPairing];
                break;
            case COMMAND_TEST_BLE_PING:
                [self doRequestCommandPing];
                break;
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                // CTAP2コマンドを生成して実行
                [self doCtap2HealthCheck];
                break;
            default:
                [self setBleRequestArray:nil];
                break;
        }
    }

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
        if (bytesBLEHeader[0] == 0x82) {
            // キープアライブの場合は引き続き次のレスポンスを待つ
            receivedData = nil;
            
        } else if (bytesBLEHeader[0] == 0x81 || bytesBLEHeader[0] == 0x83) {
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

    - (void)toolCommandWillProcessBleResponse {
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                // ステータス（１バイト）をチェック後、レスポンス処理に移る
                [[self toolCTAP2HealthCheckCommand] doCTAP2Response:[self command]
                                                    responseMessage:[self bleResponseData]];
                break;
            case COMMAND_PAIRING:
                // ステータスワード（２バイト）をチェック後、画面に制御を戻す
                [self doResponseCommandPairing:[self bleResponseData]];
                break;
            case COMMAND_TEST_REGISTER:
            case COMMAND_TEST_AUTH_CHECK:
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                // ステータスワード（２バイト）をチェック後、レスポンス処理に移る
                [[self toolU2FHealthCheckCommand] doU2FResponse:[self command]
                                                responseMessage:[self bleResponseData]];
                break;
            case COMMAND_TEST_BLE_PING:
                // PINGレスポンスの内容をチェックし、画面に制御を戻す
                [self doResponseCommandPing];
                break;
            default:
                break;
        }
    }

    - (void)commandDidProcess:(bool)result message:(NSString *)message {
        // コマンド配列をブランクに初期化
        [self setBleRequestArray:nil];
        if (message) {
            // 引数のメッセージをコンソール出力
            NSLog(@"%@", message);
            // 画面上のテキストエリアにもメッセージを表示する
            [self setLastCommandMessage:message];
        }
        // ポップアップ表示させるためのリザルトを保持
        [self setLastCommandSuccess:result];
        // デバイス接続を切断
        [[self toolBLECentral] centralManagerWillDisconnect];
    }

#pragma mark - Call back from ToolBLECentral

    - (void)centralManagerDidConnect {
        // U2F Control Pointに実行コマンドを書込
        [[self toolBLECentral] centralManagerWillSend:[self bleRequestArray]];
        [self setBleTransactionStarted:true];
    }

    - (void)centralManagerDidFailConnectionWith:(NSString *)message error:(NSError *)error {
        // BLEペアリング処理時のエラーメッセージを、適切なメッセージに変更する
        if ([self command] == COMMAND_PAIRING) {
            if ([message isEqualToString:MSG_U2F_DEVICE_SCAN_TIMEOUT]) {
                message = MSG_BLE_PARING_ERR_TIMED_OUT;
            } else if ([message isEqualToString:MSG_BLE_NOTIFICATION_FAILED]) {
                message = MSG_BLE_PARING_ERR_PAIR_MODE;
            } else if ([message isEqualToString:MSG_BLE_PARING_ERR_BT_OFF] == false) {
                message = MSG_BLE_PARING_ERR_UNKNOWN;
            }
        }
        
        // コンソールにエラーメッセージを出力
        [self displayErrorMessage:message error:error];
        // 画面上のテキストエリアにもメッセージを表示する
        [self setLastCommandMessage:message];

        // トランザクション完了済とし、接続再試行を回避
        [self setBleTransactionStarted:false];
        // ポップアップ表示させるためのリザルトを保持
        [self setLastCommandSuccess:false];
        // デバイス接続を切断
        [[self toolBLECentral] centralManagerWillDisconnect];
    }

    - (void)centralManagerDidReceive:(NSData *)bleMessage {
        if ([self isResponseCompleted:bleMessage]) {
            // 後続レスポンスがあれば、タイムアウト監視を再開させ、後続レスポンスを待つ
            [self.toolBLECentral centralManagerWillStartResponseTimeout];
        } else {
            // 後続レスポンスがなければ、トランザクション完了と判断
            [self setBleTransactionStarted:false];
            // レスポンスを次処理に引き渡す
            [self toolCommandWillProcessBleResponse];
        }
    }

    - (void)centralManagerDidDisconnectWith:(NSString *)message error:(NSError *)error {
        // コンソールにエラーメッセージを出力
        [self displayErrorMessage:message error:error];
        
        // トランザクション実行中に切断された場合は、接続を再試行（回数上限あり）
        if ([self retryBLEConnection]) {
            return;
        }
        
        // ボタンを活性化し、ポップアップメッセージを表示
        [[self delegate] bleCommandDidProcess:[self command]
                                       result:[self lastCommandSuccess]
                                      message:[self lastCommandMessage]];
    }

    - (void)notifyCentralManagerMessage:(NSString *)message {
        if (message == nil) {
            return;
        }
        // コンソールログを出力
        NSLog(@"%@", message);
    }

    - (void)notifyCentralManagerStateUpdate:(CBCentralManagerState)state {
        NSLog(@"centralManagerDidUpdateState: %ld", state);
    }

#pragma mark - Retry BLE connection

    - (bool)retryBLEConnection {
        // 処理が開始されていない場合はfalseを戻す
        if ([self bleTransactionStarted] == false) {
            return false;
        }
        
        if ([self bleConnectionRetryCount] < BLE_CONNECTION_RETRY_MAX_COUNT) {
            // 再試行回数をカウントアップ
            [self setBleConnectionRetryCount:([self bleConnectionRetryCount] + 1)];
            NSLog(MSG_BLE_CONNECTION_RETRY_WITH_CNT,
                  (unsigned long)[self bleConnectionRetryCount]);
            // BLEデバイス接続処理に移る
            [self startBleConnection];
            return true;
            
        } else {
            // 再試行上限回数に達している場合は、その旨コンソールログに出力
            NSLog(MSG_BLE_CONNECTION_RETRY_END);
            // ポップアップ表示させる失敗メッセージとリザルトを保持
            [self setLastCommandMessage:MSG_BLE_CONNECTION_RETRY_END];
            [self setLastCommandSuccess:false];
            return false;
        }
    }

#pragma mark - Common method

    - (void)displayMessage:(NSString *)string {
        // メッセージを画面表示
        [[self delegate] notifyToolCommandMessage:string];
    }

    - (void)displayStartMessage {
        // 指定コマンド種別の処理開始を通知
        [[self delegate] bleCommandStartedProcess:[self command]];
    }

    - (void)startBleConnection {
        // メッセージ表示用変数を初期化
        [self setLastCommandMessage:nil];
        [self setLastCommandSuccess:false];
        // BLEデバイス接続処理を開始する
        [self setBleTransactionStarted:false];
        [[self toolBLECentral] centralManagerWillConnect];
    }

    - (void)displayErrorMessage:(NSString *)message error:(NSError *)error {
        if (message == nil) {
            return;
        }
        // コンソールログを出力
        if (error) {
            NSLog(@"%@ %@", message, [error description]);
        } else {
            NSLog(@"%@", message);
        }
    }

#pragma mark - Interface for PinCodeParamWindow

    - (void)pinCodeParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ダイアログをモーダルで表示
        [[self toolCTAP2HealthCheckCommand] pinCodeParamWindowWillOpen:sender
                                                          parentWindow:parentWindow];
    }

    - (void)pinCodeParamWindowDidClose {
        // AppDelegateに制御を戻す（ポップアップメッセージは表示しない）
        [[self delegate] bleCommandDidProcess:COMMAND_NONE result:true message:nil];
    }

@end
