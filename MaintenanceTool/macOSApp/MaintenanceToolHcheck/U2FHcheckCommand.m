//
//  U2FHcheckCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/13.
//
#import "AppBLECommand.h"
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "AppHIDCommand.h"
#import "FIDODefines.h"
#import "ToolCommon.h"
#import "U2FHcheckCommand.h"

@interface U2FHcheckCommand () <AppHIDCommandDelegate, AppBLECommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                  delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppBLECommand            *appBLECommand;
    @property (nonatomic) AppHIDCommand            *appHIDCommand;
    // 実行対象コマンドを保持
    @property (nonatomic) Command                   command;
    // Registerレスポンスを保持（３件のテストケースで共通使用するため）
    @property (nonatomic) NSData                   *registerReponseData;
    // PINGデータを保持
    @property (nonatomic) NSData                   *pingData;
    // 使用トランスポートを保持
    @property (nonatomic) TransportType             transportType;
    // このコマンドで発生したエラーについてのメッセージを保持
    @property (nonatomic) NSString                 *errorMessage;

@end

@implementation U2FHcheckCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 上位クラスの参照を保持
            [self setDelegate:delegate];
            // ヘルパークラスのインスタンスを生成
            [self setAppBLECommand:[[AppBLECommand alloc] initWithDelegate:self]];
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self appHIDCommand] checkUSBHIDConnection];
    }

#pragma mark - Command/subcommand process

    - (void)doRequestHidU2fHealthCheck {
        // トランスポートをUSB HIDに設定
        [self setTransportType:TRANSPORT_HID];
        // CTAPHID_INITから実行
        [self setCommand:COMMAND_TEST_REGISTER];
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doRequestHidPingTest {
        // トランスポートをUSB HIDに設定
        [self setTransportType:TRANSPORT_HID];
        // CTAPHID_INITから実行
        [self setCommand:COMMAND_TEST_CTAPHID_PING];
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doResponseHIDCtap2Init {
        // CTAPHID_INIT応答後の処理を実行
        switch ([self command]) {
            case COMMAND_TEST_REGISTER:
                [self doRequestCommandRegister];
                break;
            case COMMAND_TEST_AUTH_CHECK:
                [self doRequestCommandAuthenticateCheck];
                break;
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
                [self doRequestCommandAuthenticateNoUP];
                break;
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                [self doRequestCommandAuthenticateUP];
                break;
            case COMMAND_TEST_CTAPHID_PING:
                [self doRequestCtapHidPing];
                break;
            default:
                break;
        }
    }

    - (void)doRequestCommandRegister {
        // テストデータを編集
        NSMutableData *requestData = [self createTestRequestData];
        // APDUを編集し、分割送信のために64バイトごとのコマンド配列を作成する
        NSData *dataForRequest = [self generateAPDUDataFrom:requestData INS:0x01 P1:0x00];
        // U2F Registerコマンドを実行
        [self setCommand:COMMAND_TEST_REGISTER];
        [self doRequestCtap2Command:[self command] withCMD:HID_CMD_MSG withData:dataForRequest];
    }

    - (void)doResponseCommandRegister:(NSData *)message {
        // レスポンスをチェックし、内容がNGであれば処理終了
        if ([self checkStatusWordOfResponse:message] == false) {
            [self commandDidProcess:false message:[self errorMessage]];
            return;
        }
        // 中間メッセージを表示
        [[self delegate] notifyMessage:MSG_HCHK_U2F_REGISTER_SUCCESS];
        // Registerレスポンスを内部で保持して後続処理を実行
        [self setRegisterReponseData:[[NSData alloc] initWithData:message]];
        // U2Fヘルスチェックの後続テストを実行
        [self setCommand:COMMAND_TEST_AUTH_CHECK];
        if ([self transportType] == TRANSPORT_BLE) {
            [self doRequestCommandAuthenticateCheck];
        }
        // HIDの場合は、CTAPHID_INITから再実行
        if ([self transportType] == TRANSPORT_HID) {
            [[self appHIDCommand] doRequestCtapHidInit];
        }
    }

    - (void)doRequestCommandAuthenticate:(NSData *)registerResponse P1:(unsigned char)p1 {
        // Registerレスポンスからキーハンドルを切り出し、テストデータに連結
        NSMutableData *requestData = [self createTestRequestData];
        [requestData appendData:[self getKeyHandleDataFrom:registerResponse]];
        // APDUを編集し、分割送信のために64バイトごとのコマンド配列を作成する
        NSData *dataForRequest = [self generateAPDUDataFrom:requestData INS:0x02 P1:p1];
        // U2F Authenticateコマンドを実行
        [self doRequestCtap2Command:[self command] withCMD:HID_CMD_MSG withData:dataForRequest];
    }

    - (void)doRequestCommandAuthenticateCheck {
        [self doRequestCommandAuthenticate:[self registerReponseData] P1:0x07];
    }

    - (void)doResponseCommandAuthenticateCheck:(NSData *)message {
        // レスポンスをチェックし、内容がNGであれば処理終了
        if ([self checkStatusWordOfResponse:message] == false) {
            [self commandDidProcess:false message:[self errorMessage]];
            return;
        }
        // U2Fヘルスチェックの後続テストを実行
        [self setCommand:COMMAND_TEST_AUTH_NO_USER_PRESENCE];
        if ([self transportType] == TRANSPORT_BLE) {
            [self doRequestCommandAuthenticateNoUP];
        }
        // HIDの場合は、CTAPHID_INITから再実行
        if ([self transportType] == TRANSPORT_HID) {
            [[self appHIDCommand] doRequestCtapHidInit];
        }
    }

    - (void)doRequestCommandAuthenticateNoUP {
        [self doRequestCommandAuthenticate:[self registerReponseData] P1:0x08];
    }

    - (void)doResponseCommandAuthenticateNoUP:(NSData *)message {
        // レスポンスをチェックし、内容がNGであれば処理終了
        if ([self checkStatusWordOfResponse:message] == false) {
            [self commandDidProcess:false message:[self errorMessage]];
            return;
        }
        // 後続のU2F Authenticateを開始する前に、ボタンを押してもらうように促すメッセージを表示
        [[self delegate] notifyMessage:MSG_HCHK_U2F_AUTHENTICATE_START];
        [[self delegate] notifyMessage:MSG_HCHK_U2F_AUTHENTICATE_COMMENT1];
        [[self delegate] notifyMessage:MSG_HCHK_U2F_AUTHENTICATE_COMMENT2];
        [[self delegate] notifyMessage:MSG_HCHK_U2F_AUTHENTICATE_COMMENT3];
        // U2Fヘルスチェックの後続テストを実行
        [self setCommand:COMMAND_TEST_AUTH_USER_PRESENCE];
        if ([self transportType] == TRANSPORT_BLE) {
            [self doRequestCommandAuthenticateUP];
        }
        // HIDの場合は、CTAPHID_INITから再実行
        if ([self transportType] == TRANSPORT_HID) {
            [[self appHIDCommand] doRequestCtapHidInit];
        }
    }

    - (void)doRequestCommandAuthenticateUP {
        [self doRequestCommandAuthenticate:[self registerReponseData] P1:0x03];
    }

    - (void)doResponseCommandAuthenticateUP:(NSData *)message {
        // レスポンスをチェックし、内容がNGであれば処理終了
        if ([self checkStatusWordOfResponse:message] == false) {
            [self commandDidProcess:false message:[self errorMessage]];
            return;
        }
        // 結果メッセージを表示
        [[self delegate] notifyMessage:MSG_HCHK_U2F_AUTHENTICATE_SUCCESS];
        // U2Fヘルスチェック終了
        [self setRegisterReponseData:nil];
        [self commandDidProcess:true message:nil];
    }

    - (void)doRequestCtapHidPing {
        // 100バイトのランダムなPINGデータを生成し、CTAPHID_PINGコマンドを実行
        [self setPingData:[ToolCommon generateRandomBytesDataOf:100]];
        [self doRequestCtap2Command:[self command] withCMD:HID_CMD_CTAPHID_PING withData:[self pingData]];
    }

    - (void)doResponseCtapHidPing:(NSData *)message {
        // PINGレスポンスの内容をチェックし、上位クラスに制御を戻す
        bool success = [message isEqualToData:[self pingData]];
        [self commandDidProcess:success message:MSG_CMDTST_INVALID_PING];
    }

    - (void)doResponseU2fHealthCheck:(bool)result message:(NSString *)message {
        // 上位クラスに制御を戻す
        [[self delegate] doResponseU2fHealthCheck:result message:message];
    }

#pragma mark - BLE Command/subcommand process

    - (void)doRequestBleU2fHealthCheck {
        // トランスポートをBLEに設定
        [self setTransportType:TRANSPORT_BLE];
        // U2F Registerから実行
        [self doRequestCommandRegister];
    }

    - (void)doRequestBlePingTest {
        // トランスポートをBLEに設定
        [self setTransportType:TRANSPORT_BLE];
        // BLE PING処理を実行
        [self setCommand:COMMAND_TEST_BLE_PING];
        [self doRequestCtapHidPing];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時で上位クラスに制御を戻す
        if (success == false) {
            [self doResponseU2fHealthCheck:false message:errorMessage];
            return;
        }
        // 実行コマンドにより処理分岐
        switch (command) {
            case COMMAND_HID_CTAP2_INIT:
                [self doResponseHIDCtap2Init];
                break;
            case COMMAND_TEST_REGISTER:
                [self doResponseCommandRegister:response];
                break;
            case COMMAND_TEST_AUTH_CHECK:
                [self doResponseCommandAuthenticateCheck:response];
                break;
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
                [self doResponseCommandAuthenticateNoUP:response];
                break;
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                [self doResponseCommandAuthenticateUP:response];
                break;
            case COMMAND_TEST_CTAPHID_PING:
                [self doResponseCtapHidPing:response];
                break;
            default:
                break;
        }
    }

#pragma mark - Call back from AppBLECommand

    - (void)didResponseCommand:(Command)command response:(NSData *)response {
        // 実行コマンドにより処理分岐
        switch (command) {
            case COMMAND_TEST_REGISTER:
                [self doResponseCommandRegister:response];
                break;
            case COMMAND_TEST_AUTH_CHECK:
                [self doResponseCommandAuthenticateCheck:response];
                break;
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
                [self doResponseCommandAuthenticateNoUP:response];
                break;
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                [self doResponseCommandAuthenticateUP:response];
                break;
            case COMMAND_TEST_BLE_PING:
                [self doResponseCtapHidPing:response];
                break;
            default:
                break;
        }
    }

    - (void)didCompleteCommand:(Command)command success:(bool)success errorMessage:(NSString *)errorMessage {
        // 上位クラスに制御を戻す
        [self doResponseU2fHealthCheck:success message:errorMessage];
    }

#pragma mark - Private functions

    - (void)doRequestCtap2Command:(Command)command withCMD:(uint8_t)cmd withData:(NSData *)data {
        // コマンドリクエストを、BLE／HIDトランスポート経由で実行
        if ([self transportType] == TRANSPORT_BLE) {
            [[self appBLECommand] doRequestCommand:command withCMD:cmd withData:data];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self appHIDCommand] doRequestCtap2Command:command withCMD:cmd withData:data];
        }
    }

    - (void)commandDidProcess:(bool)success message:(NSString *)message {
        // コマンド実行完了後の処理
        if ([self transportType] == TRANSPORT_BLE) {
            // 一旦ヘルパークラスに制御を戻し、BLE切断処理を実行
            [[self appBLECommand] commandDidProcess:success message:message];
        }
        if ([self transportType] == TRANSPORT_HID) {
            // 上位クラスに制御を戻す
            [self doResponseU2fHealthCheck:success message:message];
        }
    }

    - (NSMutableData *)createTestRequestData {
        // テストデータから、リクエストデータの先頭部分を生成
        NSData *challenge =
        [ToolCommon generateHexBytesFrom:
         @"124dc843bb8ba61f035a7d0938251f5dd4cbfc96f5453b130d890a1cdbae3220"];
        NSData *appIDHash =
        [ToolCommon generateHexBytesFrom:
         @"23be84e16cd6ae529049f1f1bbe9ebb3a6db3c870c3e99245e0d1c06b747deb3"];
        
        NSMutableData *requestData = [[NSMutableData alloc] initWithData:challenge];
        [requestData appendData:appIDHash];
        return requestData;
    }

    - (NSData *)generateAPDUDataFrom:(NSData *)data INS:(unsigned char)ins P1:(unsigned char)p1 {
        // APDUを編集するための一時領域
        unsigned char apduHeader[] = {0x00, ins, p1, 0x00, 0x00, 0x00, 0x00};
        unsigned char apduFooter[] = {0x00, 0x00};
        
        // リクエストデータ長を設定
        NSUInteger dataCertLength = [data length];
        apduHeader[sizeof(apduHeader)-2] = dataCertLength / 256;
        apduHeader[sizeof(apduHeader)-1] = dataCertLength % 256;
        
        // ヘッダー＋データ＋フッターを連結し、APDUを作成
        NSMutableData *dataForRequest =
        [[NSMutableData alloc] initWithBytes:apduHeader length:sizeof(apduHeader)];
        [dataForRequest appendData:data];
        [dataForRequest appendBytes:apduFooter length:sizeof(apduFooter)];
        
        return dataForRequest;
    }

    - (NSData *)getKeyHandleDataFrom:(NSData *)registerResponse {
        // Registerレスポンスからキーハンドル(67バイト目以降)を切り出し
        uint8_t *res = (uint8_t *)[registerResponse bytes];
        uint8_t keyhandleSize = res[66];
        return [registerResponse subdataWithRange:NSMakeRange(66, keyhandleSize+1)];
    }

    - (bool)checkStatusWordOfResponse:(NSData *)responseMessage {
        // レスポンスデータが揃っていない場合はNG
        if (responseMessage == nil || [responseMessage length] == 0) {
            [self setErrorMessage:MSG_OCCUR_UNKNOWN_ERROR_LEN];
            return false;
        }

        // ステータスワード(レスポンスの末尾２バイト)を取得
        NSUInteger statusWord = [self getStatusWordFrom:responseMessage];
        
        // 成功判定は、キーハンドルチェックの場合0x6985、それ以外は0x9000
        if (statusWord == 0x6985) {
            return true;
        } else if (statusWord == 0x9000) {
            return true;
        }
        
        // invalid keyhandleエラーである場合はその旨を通知
        if (statusWord == 0x6a80) {
            [self setErrorMessage:MSG_OCCUR_KEYHANDLE_ERROR];
            return false;
        }
        
        // 鍵・証明書がインストールされていない旨のエラーである場合はその旨を通知
        if (statusWord == 0x9402) {
            [self setErrorMessage:MSG_OCCUR_SKEYNOEXIST_ERROR];
            return false;
        }
        
        // ステータスワードチェックがNGの場合
        [self setErrorMessage:[NSString stringWithFormat:MSG_OCCUR_UNKNOWN_ERROR_SW, (uint16_t)statusWord]];
        return false;
    }

    - (NSUInteger)getStatusWordFrom:(NSData *)bleResponseData {
        // BLEレスポンスデータから、ステータスワードを取得する
        NSUInteger length = [bleResponseData length];
        NSData *responseStatusWord = [bleResponseData subdataWithRange:NSMakeRange(length-2, 2)];
        unsigned char *statusWordChar = (unsigned char *)[responseStatusWord bytes];
        NSUInteger statusWord = statusWordChar[0] * 256 + statusWordChar[1];
        
        return statusWord;
    }

@end
