//
//  ToolU2FHealthCheckCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/09/11.
//
#import <Foundation/Foundation.h>

#import "ToolCommon.h"
#import "ToolCommonMessage.h"
#import "ToolU2FHealthCheckCommand.h"
#import "FIDODefines.h"
#import "debug_log.h"

@interface ToolU2FHealthCheckCommand ()

    @property (nonatomic) TransportType       transportType;
    @property (nonatomic) ToolBLECommand     *toolBLECommand;
    @property (nonatomic) ToolHIDCommand     *toolHIDCommand;

    // 実行対象コマンドを保持
    @property (nonatomic) Command   command;
    // Registerレスポンスを保持（３件のテストケースで共通使用するため）
    @property (nonatomic) NSData   *registerReponseData;

@end

@implementation ToolU2FHealthCheckCommand

    - (id)init {
        self = [super init];
        NSLog(@"ToolU2FHealthCheckCommand initialized");
        return self;
    }

    - (void)setTransportParam:(TransportType)type
               toolBLECommand:(ToolBLECommand *)ble toolHIDCommand:(ToolHIDCommand *)hid {
        [self setTransportType:type];
        [self setToolBLECommand:ble];
        [self setToolHIDCommand:hid];
    }

#pragma mark - Command functions

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
        // Registerレスポンスからキーハンドル(67バイト目から65バイト)を切り出し
        return [registerResponse subdataWithRange:NSMakeRange(66, 65)];
    }

#pragma mark - Command/subcommand process

    - (void)doU2FRequest:(Command)command {
        // 実行対象コマンドを退避
        [self setCommand:command];
        switch ([self command]) {
            case COMMAND_TEST_REGISTER:
                [self doRequestCommandRegister];
                break;
            case COMMAND_TEST_AUTH_CHECK:
                [self doRequestCommandAuthenticate:[self registerReponseData] P1:0x07];
                break;
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
                [self doRequestCommandAuthenticate:[self registerReponseData] P1:0x08];
                break;
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                [self doRequestCommandAuthenticate:[self registerReponseData] P1:0x03];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、画面に制御を戻す
                [self doResponseToAppDelegate:true message:nil];
                break;
        }
    }

    - (void)doU2FResponse:(Command)command responseMessage:(NSData *)message {
        // レスポンスをチェックし、内容がNGであれば処理終了
        if ([self checkStatusWordOfResponse:message] == false) {
            [self doResponseToAppDelegate:false message:nil];
            return;
        }
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_TEST_REGISTER:
                [self doResponseCommandRegister:message];
                break;
            case COMMAND_TEST_AUTH_CHECK:
                [self doResponseCommandAuthenticateCheck:message];
                break;
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
                [self doResponseCommandAuthenticateNoUP:message];
                break;
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                [self doResponseCommandAuthenticateUP:message];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、画面に制御を戻す
                [self doResponseToAppDelegate:false message:nil];
                break;
        }
    }

    - (bool)checkStatusWordOfResponse:(NSData *)responseMessage {
        // レスポンスデータが揃っていない場合はNG
        if (responseMessage == nil || [responseMessage length] == 0) {
            [self displayMessage:MSG_OCCUR_UNKNOWN_ERROR];
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
            [self displayMessage:MSG_OCCUR_KEYHANDLE_ERROR];
            return false;
        }
        
        // 鍵・証明書がインストールされていない旨のエラーである場合はその旨を通知
        if (statusWord == 0x9402) {
            [self displayMessage:MSG_OCCUR_SKEYNOEXIST_ERROR];
            return false;
        }
        
        // ペアリングモード時はペアリング以外の機能を実行できない旨を通知
        if (statusWord == 0x9601) {
            [self displayMessage:MSG_OCCUR_PAIRINGMODE_ERROR];
            return false;
        }
        
        // ステータスワードチェックがNGの場合
        [self displayMessage:MSG_OCCUR_UNKNOWN_BLE_ERROR];
        return false;
    }

    - (void)doRequestCommandRegister {
        NSLog(@"Health check start");
        
        // テストデータを編集
        NSMutableData *requestData = [self createTestRequestData];
        // APDUを編集し、分割送信のために64バイトごとのコマンド配列を作成する
        NSData *dataForRequest = [self generateAPDUDataFrom:requestData INS:0x01 P1:0x00];
        // U2F Registerコマンドを実行
        if ([self transportType] == TRANSPORT_BLE) {
            [[self toolBLECommand] doBLECommandRequestFrom:dataForRequest cmd:BLE_CMD_MSG];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] doRequest:dataForRequest CID:[self CID] CMD:HID_CMD_MSG];
        }
    }

    - (void)doResponseCommandRegister:(NSData *)message {
        // 中間メッセージを表示
        [self displayMessage:MSG_HCHK_U2F_REGISTER_SUCCESS];
        NSLog(@"Register test success");
        // Registerレスポンスを内部で保持して後続処理を実行
        [self setRegisterReponseData:[[NSData alloc] initWithData:message]];
        // U2Fヘルスチェックの後続テストを実行
        if ([self transportType] == TRANSPORT_BLE) {
            [self setCommand:COMMAND_TEST_AUTH_CHECK];
            [self doRequestCommandAuthenticate:[self registerReponseData] P1:0x07];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TEST_AUTH_CHECK];
        }
    }

    - (void)doRequestCommandAuthenticate:(NSData *)registerResponse P1:(unsigned char)p1 {
        // Registerレスポンスからキーハンドルを切り出し、テストデータに連結
        NSMutableData *requestData = [self createTestRequestData];
        [requestData appendData:[self getKeyHandleDataFrom:registerResponse]];
        // APDUを編集し、分割送信のために64バイトごとのコマンド配列を作成する
        NSData *dataForRequest = [self generateAPDUDataFrom:requestData INS:0x02 P1:p1];
        // U2F Authenticateコマンドを実行
        if ([self transportType] == TRANSPORT_BLE) {
            [[self toolBLECommand] doBLECommandRequestFrom:dataForRequest cmd:BLE_CMD_MSG];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] doRequest:dataForRequest CID:[self CID] CMD:HID_CMD_MSG];
        }
    }

    - (void)doResponseCommandAuthenticateCheck:(NSData *)message {
        // 中間メッセージを表示
        NSLog(@"Authenticate test (check) success");
        // U2Fヘルスチェックの後続テストを実行
        if ([self transportType] == TRANSPORT_BLE) {
            [self setCommand:COMMAND_TEST_AUTH_NO_USER_PRESENCE];
            [self doRequestCommandAuthenticate:[self registerReponseData] P1:0x08];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TEST_AUTH_NO_USER_PRESENCE];
        }
    }

    - (void)doResponseCommandAuthenticateNoUP:(NSData *)message {
        // 中間メッセージを表示
        NSLog(@"Authenticate test (dont-enforce-user-presence-and-sign) success");
        // 後続のU2F Authenticateを開始する前に、
        // 基板上のMAIN SWを押してもらうように促すメッセージを表示
        [self displayMessage:MSG_HCHK_U2F_AUTHENTICATE_START];
        [self displayMessage:MSG_HCHK_U2F_AUTHENTICATE_COMMENT1];
        [self displayMessage:MSG_HCHK_U2F_AUTHENTICATE_COMMENT2];
        [self displayMessage:MSG_HCHK_U2F_AUTHENTICATE_COMMENT3];
        // U2Fヘルスチェックの後続テストを実行
        if ([self transportType] == TRANSPORT_BLE) {
            [self setCommand:COMMAND_TEST_AUTH_USER_PRESENCE];
            [self doRequestCommandAuthenticate:[self registerReponseData] P1:0x03];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TEST_AUTH_USER_PRESENCE];
        }
    }

    - (void)doResponseCommandAuthenticateUP:(NSData *)message {
        // 結果メッセージを表示
        NSLog(@"Authenticate test (enforce-user-presence-and-sign) success");
        [self displayMessage:MSG_HCHK_U2F_AUTHENTICATE_SUCCESS];
        // U2Fヘルスチェック終了
        [self doResponseToAppDelegate:true message:@"Health check end"];
        [self setRegisterReponseData:nil];
    }

#pragma mark - Common methods

    - (void)displayMessage:(NSString *)message {
        // メッセージを画面表示
        if ([self transportType] == TRANSPORT_BLE) {
            [[self toolBLECommand] displayMessage:message];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] displayMessage:message];
        }
    }

    - (void)doResponseToAppDelegate:(bool)result message:(NSString *)message {
        if ([self transportType] == TRANSPORT_BLE) {
            // BLE接続を切断してから、アプリケーションに制御を戻す
            [[self toolBLECommand] commandDidProcess:result message:message];
        }
        if ([self transportType] == TRANSPORT_HID) {
            // 即時でアプリケーションに制御を戻す
            [[self toolHIDCommand] commandDidProcess:result message:message];
        }
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
