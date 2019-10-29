//
//  ToolHIDCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/03/20.
//
#import <Foundation/Foundation.h>

#import "ToolCommonMessage.h"
#import "ToolHIDCommand.h"
#import "ToolHIDHelper.h"
#import "ToolInstallCommand.h"
#import "ToolClientPINCommand.h"
#import "ToolCTAP2HealthCheckCommand.h"
#import "ToolU2FHealthCheckCommand.h"
#import "ToolPopupWindow.h"
#import "FIDODefines.h"

@interface ToolHIDCommand () <ToolHIDHelperDelegate>

    @property (nonatomic) ToolHIDHelper        *toolHIDHelper;
    @property (nonatomic) ToolInstallCommand   *toolInstallCommand;
    @property (nonatomic) ToolClientPINCommand *toolClientPINCommand;
    @property (nonatomic) ToolCTAP2HealthCheckCommand
                                               *toolCTAP2HealthCheckCommand;
    @property (nonatomic) ToolU2FHealthCheckCommand
                                               *toolU2FHealthCheckCommand;

    @property (nonatomic) Command   command;
    @property (nonatomic) NSString *skeyFilePath;
    @property (nonatomic) NSString *certFilePath;

    // 送信PINGデータを保持
    @property(nonatomic) NSData    *pingData;
    // 処理機能名称を保持
    @property(nonatomic) NSString  *processNameOfCommand;

@end

@implementation ToolHIDCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolHIDCommandDelegate>)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
        }
        [self setToolHIDHelper:[[ToolHIDHelper alloc] initWithDelegate:self]];
        [self setToolInstallCommand:[[ToolInstallCommand alloc] init]];
        [self setToolClientPINCommand:[[ToolClientPINCommand alloc] init]];
        [self setToolCTAP2HealthCheckCommand:[[ToolCTAP2HealthCheckCommand alloc] init]];
        [[self toolCTAP2HealthCheckCommand] setTransportParam:TRANSPORT_HID
                                               toolBLECommand:nil
                                               toolHIDCommand:self];
        [self setToolU2FHealthCheckCommand:[[ToolU2FHealthCheckCommand alloc] init]];
        [[self toolU2FHealthCheckCommand] setTransportParam:TRANSPORT_HID
                                             toolBLECommand:nil
                                             toolHIDCommand:self];
        return self;
    }

#pragma mark - Response timeout monitor

    - (void)startTimeoutMonitorForSelector:(SEL)selector withObject:object afterDelay:(NSTimeInterval)delay {
        [self cancelTimeoutMonitorForSelector:selector withObject:object];
        [self performSelector:selector withObject:object afterDelay:delay];
    }

    - (void)cancelTimeoutMonitorForSelector:(SEL)selector withObject:object {
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:selector object:object];
    }

    - (void)startResponseTimeoutMonitor {
        // タイムアウト監視を開始（30秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(responseTimeoutMonitorDidTimeout)
                                  withObject:nil afterDelay:30.0];
        // for debug
        // NSLog(@"ResponseTimeoutMonitor started");
    }

    - (void)cancelResponseTimeoutMonitor {
        // タイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(responseTimeoutMonitorDidTimeout)
                                   withObject:nil];
        // for debug
        // NSLog(@"ResponseTimeoutMonitor canceled");
    }

    - (void)responseTimeoutMonitorDidTimeout {
        // タイムアウト時はエラーメッセージを表示
        NSLog(@"HIDResponse timed out");
        [[self delegate] hidCommandDidProcess:[self processNameOfCommand]
                                       result:false message:MSG_HID_CMD_RESPONSE_TIMEOUT];
    }

#pragma mark - Parameter set

    - (void)setInstallParameter:(Command)command
                   skeyFilePath:(NSString *)skeyFilePath certFilePath:(NSString *)certFilePath {
        // インストール対象の鍵・証明書ファイルパスを保持
        [self setSkeyFilePath:skeyFilePath];
        [self setCertFilePath:certFilePath];
    }

#pragma mark - Constants for test

    char cidBytes[] = {0xff, 0xff, 0xff, 0xff};
    char nonceBytes[] = {0x71, 0xcb, 0x1c, 0x3b, 0x10, 0x8e, 0xc9, 0x24};

#pragma mark - Command functions

    - (void)doRequest:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd {
        // HIDデバイスにリクエストを送信
        [[self toolHIDHelper] hidHelperWillSend:message CID:cid CMD:cmd];
        // レスポンスタイムアウトを監視
        [self startResponseTimeoutMonitor];
    }

    - (void)doRequestCtapHidInit {
        // CTAPHID_INITコマンドを実行
        NSData *message = [[NSData alloc] initWithBytes:nonceBytes length:sizeof(nonceBytes)];
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [self doRequest:message CID:cid CMD:HID_CMD_CTAPHID_INIT];
    }

    - (void)doResponseCtapHidInit:(NSData *)message {
        if ([self isCorrectNonceBytes:message] == false) {
            // レスポンスメッセージのnonceと、リクエスト時のnonceが一致していない場合は、
            // 画面に制御を戻す
            [self commandDidProcess:false message:nil];
        }
        switch ([self command]) {
            case COMMAND_TEST_CTAPHID_PING:
                // 受領したCIDを使用し、CTAPHID_PINGコマンドを実行
                [self doRequestCtapHidPing:[self getNewCIDFrom:message]];
                break;
            case COMMAND_CLIENT_PIN_SET:
            case COMMAND_CLIENT_PIN_CHANGE:
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
            case COMMAND_AUTH_RESET:
                // 受領したCIDを使用し、GetKeyAgreement／authenticatorResetコマンドを実行
                [[self toolCTAP2HealthCheckCommand] setCID:[self getNewCIDFrom:message]];
                [[self toolCTAP2HealthCheckCommand] doCTAP2Request:[self command]];
                break;
            case COMMAND_TEST_REGISTER:
            case COMMAND_TEST_AUTH_CHECK:
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                // 受領したCIDを使用し、U2FRegister／Authenticateコマンドを実行
                [[self toolU2FHealthCheckCommand] setCID:[self getNewCIDFrom:message]];
                [[self toolU2FHealthCheckCommand] doU2FRequest:[self command]];
                break;
            default:
                // 画面に制御を戻す
                NSLog(@"Unknown command %ld", (long)[self command]);
                [self commandDidProcess:false message:nil];
                break;
        }
    }

    - (void)doTestCtapHidPing {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doRequestCtapHidPing:(NSData *)cid {
        // 100バイトのランダムなPINGデータを生成
        [self setPingData:[ToolCommon generateRandomBytesDataOf:100]];
        // メッセージを編集し、CTAPHID_PINGコマンドを実行
        [self doRequest:[self pingData] CID:cid CMD:HID_CMD_CTAPHID_PING];
    }

    - (void)doResponseCtapHidPing:(NSData *)message {
        // PINGレスポンスの内容をチェックし、画面に制御を戻す
        bool result = [message isEqualToData:[self pingData]];
        [self commandDidProcess:result message:MSG_CMDTST_INVALID_PING];
    }

    - (void)doHidGetFlashStat {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        // コマンド 0xC2 を実行（メッセージはブランクとする）
        NSData *message = [[NSData alloc] init];
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [self doRequest:message CID:cid CMD:HID_CMD_GET_FLASH_STAT];
    }

    - (void)doResponseHidGetFlashStat:(NSData *)message {
        // 戻りメッセージから、取得情報CSVを抽出
        NSData *responseBytes = [self extractCBORBytesFrom:message];
        NSString *responseCSV = [[NSString alloc] initWithData:responseBytes encoding:NSASCIIStringEncoding];
        NSLog(@"Flash ROM statistics: %@", responseCSV);
        // 情報取得CSVから空き領域に関する情報を抽出
        NSString *strRemain = @"";
        NSString *strAvail = @"";
        NSString *strCorrupt = @"";
        for (NSString *element in [responseCSV componentsSeparatedByString:@","]) {
            NSArray *items = [element componentsSeparatedByString:@"="];
            NSString *key = [items objectAtIndex:0];
            NSString *val = [items objectAtIndex:1];
            if ([key isEqualToString:@"largest_contig"]) {
                strRemain = val;
            } else if ([key isEqualToString:@"words_available"]) {
                strAvail = val;
            } else if ([key isEqualToString:@"corruption"]) {
                strCorrupt = val;
            }
        }
        // 空き容量、破損状況を画面に表示
        NSString *rateText;
        if ([strRemain length] > 0 && [strAvail length] > 0) {
            float rate = [strRemain floatValue] / [strAvail floatValue] * 100.0;
            rateText = [NSString stringWithFormat:MSG_FSTAT_REMAINING_RATE, rate];
        } else {
            rateText = MSG_FSTAT_NON_REMAINING_RATE;
        }
        NSString *corruptText = [strCorrupt isEqualToString:@"0"] ?
            MSG_FSTAT_CORRUPTING_AREA_NOT_EXIST : MSG_FSTAT_CORRUPTING_AREA_EXIST;
        // 画面に制御を戻す
        [self displayMessage:[NSString stringWithFormat:@"  %1$@%2$@", rateText, corruptText]];
        [self commandDidProcess:true message:nil];
    }

    - (void)doHidGetVersionInfo {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        // コマンド 0xC3 を実行（メッセージはブランクとする）
        NSData *message = [[NSData alloc] init];
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [self doRequest:message CID:cid CMD:HID_CMD_GET_VERSION_INFO];
    }

    - (void)doResponseHidGetVersionInfo:(NSData *)message {
        // 戻りメッセージから、取得情報CSVを抽出
        NSData *responseBytes = [self extractCBORBytesFrom:message];
        NSString *responseCSV = [[NSString alloc] initWithData:responseBytes encoding:NSASCIIStringEncoding];
        NSLog(@"FIDO authenticator version info: %@", responseCSV);
        // 情報取得CSVからバージョン情報を抽出
        NSString *strDeviceName = @"";
        NSString *strFWRev = @"";
        NSString *strHWRev = @"";
        for (NSString *element in [responseCSV componentsSeparatedByString:@","]) {
            NSArray *items = [element componentsSeparatedByString:@"="];
            NSString *key = [items objectAtIndex:0];
            NSString *val = [items objectAtIndex:1];
            if ([key isEqualToString:@"DEVICE_NAME"]) {
                strDeviceName = [self extractCSVItemFrom:val];
            } else if ([key isEqualToString:@"FW_REV"]) {
                strFWRev = [self extractCSVItemFrom:val];
            } else if ([key isEqualToString:@"HW_REV"]) {
                strHWRev = [self extractCSVItemFrom:val];
            }
        }
        // 画面に制御を戻す
        [self displayMessage:MSG_VERSION_INFO_HEADER];
        [self displayMessage:[NSString stringWithFormat:MSG_VERSION_INFO_DEVICE_NAME, strDeviceName]];
        [self displayMessage:[NSString stringWithFormat:MSG_VERSION_INFO_FW_REV, strFWRev]];
        [self displayMessage:[NSString stringWithFormat:MSG_VERSION_INFO_HW_REV, strHWRev]];
        [self commandDidProcess:true message:nil];
    }

    - (void)doEraseSkeyCert {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        // メッセージを編集し、コマンド 0xC0 を実行
        NSData *message = [[self toolInstallCommand] generateEraseSkeyCertMessage:[self command]];
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [self doRequest:message CID:cid CMD:HID_CMD_ERASE_SKEY_CERT];
    }

    - (void)doInstallSkeyCert {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        // メッセージを編集
        NSData *message = [[self toolInstallCommand] generateInstallSkeyCertMessage:[self command]
                            skeyFilePath:[self skeyFilePath] certFilePath:[self certFilePath]];
        if (message == nil) {
            // 処理が失敗した場合は、AppDelegateに制御を戻す
            [self commandDidProcess:false message:[[self toolInstallCommand] lastErrorMessage]];
            return;
        }

        // コマンド 0xC1 を実行
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [self doRequest:message CID:cid CMD:HID_CMD_INSTALL_SKEY_CERT];
    }

    - (void)doResponseMaintenanceCommand:(NSData *)message {
        // ステータスコードを確認し、画面に制御を戻す
        [self commandDidProcess:[[self toolCTAP2HealthCheckCommand] checkStatusCode:message] message:nil];
    }

    - (void)doClientPin {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doClientPinSetOrChange:(NSData *)message CID:(NSData *)cid {
        // メッセージを編集し、GetKeyAgreementサブコマンドを実行
        NSData *request = [[self toolClientPINCommand] generateClientPinSetRequestWith:message];
        if (request == nil) {
            [self commandDidProcess:false message:nil];
            return;
        }
        // コマンドを実行
        [self doRequest:request CID:cid CMD:HID_CMD_CTAPHID_CBOR];
    }

    - (void)doCtap2HealthCheck {
        // コマンド開始メッセージを画面表示
        if ([self command] == COMMAND_TEST_MAKE_CREDENTIAL) {
            [self displayStartMessage];
        }
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doResponseCtapHidCbor:(NSData *)message
                            CID:(NSData *)cid CMD:(uint8_t)cmd {
        // ステータスコードを確認し、NGの場合は画面に制御を戻す
        [[self toolCTAP2HealthCheckCommand] setCID:cid];
        [[self toolCTAP2HealthCheckCommand] doCTAP2Response:[self command] responseMessage:message];
    }

    - (void)doU2FHealthCheck {
        // コマンド開始メッセージを画面表示
        if ([self command] == COMMAND_TEST_REGISTER) {
            [self displayStartMessage];
        }
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doResponseU2FHidMsg:(NSData *)message
                            CID:(NSData *)cid CMD:(uint8_t)cmd {
        // ステータスコードを確認し、NGの場合は画面に制御を戻す
        [[self toolU2FHealthCheckCommand] setCID:cid];
        [[self toolU2FHealthCheckCommand] doU2FResponse:[self command] responseMessage:message];
    }

    - (void)hidHelperWillProcess:(Command)command {
        // コマンドを待避
        [self setCommand:command];
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_TEST_CTAPHID_PING:
                [self doTestCtapHidPing];
                break;
            case COMMAND_HID_GET_FLASH_STAT:
                [self doHidGetFlashStat];
                break;
            case COMMAND_HID_GET_VERSION_INFO:
                [self doHidGetVersionInfo];
                break;
            case COMMAND_ERASE_SKEY_CERT:
                [self doEraseSkeyCert];
                break;
            case COMMAND_INSTALL_SKEY_CERT:
                [self doInstallSkeyCert];
                break;
            case COMMAND_CLIENT_PIN_SET:
            case COMMAND_CLIENT_PIN_CHANGE:
            case COMMAND_AUTH_RESET:
                [self doClientPin];
                break;
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                [self doCtap2HealthCheck];
                break;
            case COMMAND_TEST_REGISTER:
            case COMMAND_TEST_AUTH_CHECK:
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                [self doU2FHealthCheck];
                break;
            default:
                // エラーメッセージを表示
                [ToolPopupWindow critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil];
                [[self delegate] hidCommandDidProcess:[self processNameOfCommand] result:false message:nil];
                break;
        }
    }

#pragma mark - Call back from ToolHIDHelper

    - (void)hidHelperDidReceive:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd {
        // レスポンスタイムアウト監視を停止
        [self cancelResponseTimeoutMonitor];
        // コマンドに応じ、以下の処理に分岐
        switch (cmd) {
            case HID_CMD_CTAPHID_PING:
                [self doResponseCtapHidPing:message];
                break;
            case HID_CMD_CTAPHID_INIT:
                [self doResponseCtapHidInit:message];
                break;
            case HID_CMD_GET_FLASH_STAT:
                [self doResponseHidGetFlashStat:message];
                break;
            case HID_CMD_GET_VERSION_INFO:
                [self doResponseHidGetVersionInfo:message];
                break;
            case HID_CMD_ERASE_SKEY_CERT:
            case HID_CMD_INSTALL_SKEY_CERT:
                [self doResponseMaintenanceCommand:message];
                break;
            case HID_CMD_CTAPHID_CBOR:
                [self doResponseCtapHidCbor:message CID:cid CMD:cmd];
                break;
            case HID_CMD_MSG:
                [self doResponseU2FHidMsg:message CID:cid CMD:cmd];
                break;
            case HID_CMD_UNKNOWN_ERROR:
                // メッセージを画面表示
                [self commandDidProcess:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
            default:
                break;
        }
    }

#pragma mark - Common method

    - (void)displayMessage:(NSString *)string {
        // メッセージを画面表示
        [[self delegate] notifyToolCommandMessage:string];
    }

    - (void)displayStartMessage {
        // コマンド種別に対応する処理名称を設定
        switch ([self command]) {
            case COMMAND_ERASE_SKEY_CERT:
                [self setProcessNameOfCommand:PROCESS_NAME_ERASE_SKEY_CERT];
                break;
            case COMMAND_INSTALL_SKEY_CERT:
                [self setProcessNameOfCommand:PROCESS_NAME_INSTALL_SKEY_CERT];
                break;
            case COMMAND_TEST_CTAPHID_PING:
                [self setProcessNameOfCommand:PROCESS_NAME_TEST_CTAPHID_PING];
                break;
            case COMMAND_HID_GET_FLASH_STAT:
                [self setProcessNameOfCommand:PROCESS_NAME_GET_FLASH_STAT];
                break;
            case COMMAND_HID_GET_VERSION_INFO:
                [self setProcessNameOfCommand:PROCESS_NAME_GET_VERSION_INFO];
                break;
            case COMMAND_CLIENT_PIN_SET:
                [self setProcessNameOfCommand:PROCESS_NAME_CLIENT_PIN_SET];
                break;
            case COMMAND_CLIENT_PIN_CHANGE:
                [self setProcessNameOfCommand:PROCESS_NAME_CLIENT_PIN_CHANGE];
                break;
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                [self setProcessNameOfCommand:PROCESS_NAME_HID_CTAP2_HEALTHCHECK];
                break;
            case COMMAND_AUTH_RESET:
                [self setProcessNameOfCommand:PROCESS_NAME_AUTH_RESET];
                break;
            case COMMAND_TEST_REGISTER:
            case COMMAND_TEST_AUTH_CHECK:
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                [self setProcessNameOfCommand:PROCESS_NAME_HID_U2F_HEALTHCHECK];
                break;
            default:
                [self setProcessNameOfCommand:nil];
                break;
        }
        // コマンド開始メッセージを画面表示
        NSString *startMsg = [NSString stringWithFormat:MSG_FORMAT_START_MESSAGE,
                              [self processNameOfCommand]];
        [self displayMessage:startMsg];
    }

    - (void)commandDidProcess:(bool)result message:(NSString *)message {
        // 即時でアプリケーションに制御を戻す
        [[self delegate] hidCommandDidProcess:[self processNameOfCommand] result:result message:message];
    }

    - (NSData *)extractCBORBytesFrom:(NSData *)responseMessage {
        // CBORバイト配列（レスポンスの２バイト目以降）を抽出
        size_t cborLength = [responseMessage length] - 1;
        NSData *cborBytes = [responseMessage subdataWithRange:NSMakeRange(1, cborLength)];
        return cborBytes;
    }

    - (NSString *)extractCSVItemFrom:(NSString *)val {
        // 文字列の前後に２重引用符が含まれていない場合は終了
        if ([val length] < 2) {
            return val;
        }
        // 取得した項目から、２重引用符を削除
        NSString *item = [val stringByReplacingOccurrencesOfString:@"\"" withString:@""];
        return item;
    }

    - (NSData *)getNewCIDFrom:(NSData *)hidInitResponseMessage {
        // CTAPHID_INITレスポンスからCID（9〜12バイト目）を抽出
        NSData *newCID = [hidInitResponseMessage subdataWithRange:NSMakeRange(8, 4)];
        return newCID;
    }

    - (bool)isCorrectNonceBytes:(NSData *)hidInitResponseMessage {
        // レスポンスメッセージのnonce（先頭8バイト）と、リクエスト時のnonceが一致しているか確認
        char *responseBytes = (char *)[hidInitResponseMessage bytes];
        return (memcmp(responseBytes, nonceBytes, sizeof(nonceBytes)) == 0);
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        if (![[self toolHIDHelper] isDeviceConnected]) {
            [ToolPopupWindow critical:MSG_CMDTST_PROMPT_USB_PORT_SET informativeText:nil];
            return false;
        }
        return true;
    }

#pragma mark - Interface for SetPinParamWindow

    - (void)setPinParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ダイアログをモーダルで表示
        [[self toolClientPINCommand] setPinParamWindowWillOpen:sender
                                                  parentWindow:parentWindow toolCommand:self];
    }

    - (void)setPinParamWindowDidClose {
        // AppDelegateに制御を戻す（ポップアップメッセージは表示しない）
        [[self delegate] hidCommandDidProcess:nil result:true message:nil];
    }

#pragma mark - Interface for PinCodeParamWindow

    - (void)pinCodeParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ダイアログをモーダルで表示
        [[self toolCTAP2HealthCheckCommand] pinCodeParamWindowWillOpen:sender
                                            parentWindow:parentWindow];
    }

    - (void)pinCodeParamWindowDidClose {
        // AppDelegateに制御を戻す（ポップアップメッセージは表示しない）
        [[self delegate] hidCommandDidProcess:nil result:true message:nil];
    }

@end
