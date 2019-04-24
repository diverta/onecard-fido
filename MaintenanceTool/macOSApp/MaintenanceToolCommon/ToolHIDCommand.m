//
//  ToolHIDCommand.m
//  U2FMaintenanceTool
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
#import "ToolPopupWindow.h"
#import "FIDODefines.h"

// HIDコマンドバイト
#define HID_CMD_CTAPHID_INIT        0x86
#define HID_CMD_CTAPHID_CBOR        0x90
#define HID_CMD_ERASE_SKEY_CERT     0xC0
#define HID_CMD_INSTALL_SKEY_CERT   0xC1

@interface ToolHIDCommand () <ToolHIDHelperDelegate>

    @property (nonatomic) ToolHIDHelper        *toolHIDHelper;
    @property (nonatomic) ToolInstallCommand   *toolInstallCommand;
    @property (nonatomic) ToolClientPINCommand *toolClientPINCommand;
    @property (nonatomic) ToolCTAP2HealthCheckCommand
                                               *toolCTAP2HealthCheckCommand;

    @property (nonatomic) Command   command;
    @property (nonatomic) NSString *skeyFilePath;
    @property (nonatomic) NSString *certFilePath;

    // 実行中のサブコマンドを保持
    @property (nonatomic) uint8_t   cborCommand;
    @property (nonatomic) uint8_t   subCommand;

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
        // タイムアウト監視を開始（10秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(responseTimeoutMonitorDidTimeout)
                                  withObject:nil afterDelay:10.0];
        NSLog(@"ResponseTimeoutMonitor started");
    }

    - (void)cancelResponseTimeoutMonitor {
        // タイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(responseTimeoutMonitorDidTimeout)
                                   withObject:nil];
        NSLog(@"ResponseTimeoutMonitor canceled");
    }

    - (void)responseTimeoutMonitorDidTimeout {
        // タイムアウト時はエラーメッセージを表示
        [[self delegate] hidCommandDidProcess:[self command]
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

    - (void)displayMessage:(NSString *)string {
        // メッセージを画面表示
        [[self delegate] notifyToolCommandMessage:string];
    }

    - (void)displayStartMessage {
        // コマンド開始メッセージを画面表示
        NSString *startMsg = [NSString stringWithFormat:MSG_FORMAT_START_MESSAGE,
                              [ToolCommon processNameOfCommand:[self command]]];
        [self displayMessage:startMsg];
    }

    - (void)doRequest:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd {
        // HIDデバイスにリクエストを送信
        [[self toolHIDHelper] hidHelperWillSend:message CID:cid CMD:cmd];
        // レスポンスタイムアウトを監視
        [self startResponseTimeoutMonitor];
    }

    - (void)doResponseToAppDelegate:(bool)result message:(NSString *)message {
        // AppDelegateに制御を戻す
        [[self delegate] hidCommandDidProcess:[self command] result:result message:message];
    }

    - (void)doRequestCtapHidInit {
        // CTAPHID_INITコマンドを実行
        NSData *message = [[NSData alloc] initWithBytes:nonceBytes length:sizeof(nonceBytes)];
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [self doRequest:message CID:cid CMD:HID_CMD_CTAPHID_INIT];
    }

    - (void)doTestCtapHidInit {
        // CTAPHID_INITコマンドをテスト実行する
        [self doRequestCtapHidInit];
    }

    - (void)doResponseCtapHidInit:(NSData *)message {
        if ([self isCorrectNonceBytes:message] == false) {
            // レスポンスメッセージのnonceと、リクエスト時のnonceが一致していない場合は、
            // 画面に制御を戻す
            [self doResponseToAppDelegate:false message:nil];
        }
        switch ([self command]) {
            case COMMAND_TEST_CTAPHID_INIT:
                // 画面に制御を戻す
                [self doResponseToAppDelegate:true message:nil];
                break;
            case COMMAND_CLIENT_PIN_SET:
            case COMMAND_CLIENT_PIN_CHANGE:
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                // 受領したCIDを使用し、GetKeyAgreementコマンドを実行
                [self setCborCommand:CTAP2_CMD_CLIENT_PIN];
                [self setSubCommand:CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT];
                [self doRequestGetKeyAgreement:[self getNewCIDFrom:message]];
                break;
            default:
                break;
        }
    }

    - (bool)isCorrectNonceBytes:(NSData *)hidInitResponseMessage {
        // レスポンスメッセージのnonce（先頭8バイト）と、リクエスト時のnonceが一致しているか確認
        char *responseBytes = (char *)[hidInitResponseMessage bytes];
        return (memcmp(responseBytes, nonceBytes, sizeof(nonceBytes)) == 0);
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
            [self doResponseToAppDelegate:false message:[[self toolInstallCommand] lastErrorMessage]];
            return;
        }

        // コマンド 0xC1 を実行
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [self doRequest:message CID:cid CMD:HID_CMD_INSTALL_SKEY_CERT];
    }

    - (void)doResponseMaintenanceCommand:(NSData *)message {
        // ステータスコードを確認し、画面に制御を戻す
        [self doResponseToAppDelegate:[self checkStatusCode:message] message:nil];
    }

    - (void)doClientPin {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doResponseCtapHidCbor:(NSData *)message
                            CID:(NSData *)cid CMD:(uint8_t)cmd {
        // ステータスコードを確認し、NGの場合は画面に制御を戻す
        if ([self checkStatusCode:message] == false) {
            [self doResponseToAppDelegate:false message:nil];
            return;
        }
        switch ([self cborCommand]) {
            case CTAP2_CMD_CLIENT_PIN:
                [self doResponseCommandClientPin:message CID:cid CMD:cmd];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、画面に制御を戻す
                [self doResponseToAppDelegate:false message:nil];
                break;
        }
    }

    - (void)doResponseCommandClientPin:(NSData *)message
                                   CID:(NSData *)cid CMD:(uint8_t)cmd {
        // レスポンスされたCBORを抽出
        NSData *cborBytes = [self extractCBORBytesFrom:message];
        
        switch ([self subCommand]) {
            case CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT:
                [self doResponseCommandGetKeyAgreement:cborBytes CID:cid];
                break;
            default:
                // 画面に制御を戻す
                [self doResponseToAppDelegate:true message:nil];
                break;
        }
    }

    - (void)doResponseCommandGetKeyAgreement:(NSData *)message CID:(NSData *)cid {
        switch ([self command]) {
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                // PINトークン取得処理を続行
                [self doClientPinTokenGet:message CID:cid];
                break;
            default:
                // PIN設定処理を続行
                [self doClientPinSetOrChange:message CID:cid];
                break;
        }
    }

    - (void)doClientPinSetOrChange:(NSData *)message CID:(NSData *)cid {
        // 実行するコマンドを退避
        [self setCborCommand:CTAP2_CMD_CLIENT_PIN];
        if ([[[self toolClientPINCommand] pinOld] length] == 0) {
            [self setSubCommand:CTAP2_SUBCMD_CLIENT_PIN_SET];
        } else {
            [self setSubCommand:CTAP2_SUBCMD_CLIENT_PIN_CHANGE];
        }
        // メッセージを編集し、GetKeyAgreementサブコマンドを実行
        NSData *request = [[self toolClientPINCommand] generateClientPinSetRequestWith:message];
        if (request == nil) {
            [self doResponseToAppDelegate:false message:nil];
            return;
        }
        // コマンドを実行
        [self doRequest:request CID:cid CMD:HID_CMD_CTAPHID_CBOR];
    }

    - (NSData *)extractCBORBytesFrom:(NSData *)responseMessage {
        // CBORバイト配列（レスポンスの２バイト目以降）を抽出
        size_t cborLength = [responseMessage length] - 1;
        NSData *cborBytes = [responseMessage subdataWithRange:NSMakeRange(1, cborLength)];
        return cborBytes;
    }

    - (NSData *)getNewCIDFrom:(NSData *)hidInitResponseMessage {
        // CTAPHID_INITレスポンスからCID（9〜12バイト目）を抽出
        NSData *newCID = [hidInitResponseMessage subdataWithRange:NSMakeRange(8, 4)];
        return newCID;
    }

    - (void)doRequestGetKeyAgreement:(NSData *)cid {
        // メッセージを編集し、GetKeyAgreementサブコマンドを実行
        NSData *message = [[self toolClientPINCommand] generateGetKeyAgreementRequest:[self command]];
        if (message == nil) {
            [self doResponseToAppDelegate:false message:nil];
            return;
        }
        [self doRequest:message CID:cid CMD:HID_CMD_CTAPHID_CBOR];
    }

    - (void)doClientPinTokenGet:(NSData *)message CID:(NSData *)cid {
        // 実行するコマンドを退避
        [self setCborCommand:CTAP2_CMD_CLIENT_PIN];
        [self setSubCommand:CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN];
        // メッセージを編集し、getPinTokenサブコマンドを実行
        NSData *request = [[self toolCTAP2HealthCheckCommand]
                                generateClientPinTokenGetRequestWith:message];
        if (request == nil) {
            [self doResponseToAppDelegate:false message:nil];
            return;
        }
        // コマンドを実行
        [self doRequest:request CID:cid CMD:HID_CMD_CTAPHID_CBOR];
    }

    - (void)hidHelperWillProcess:(Command)command {
        // コマンドを待避
        [self setCommand:command];
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_TEST_CTAPHID_INIT:
                [self doTestCtapHidInit];
                break;
            case COMMAND_ERASE_SKEY_CERT:
                [self doEraseSkeyCert];
                break;
            case COMMAND_INSTALL_SKEY_CERT:
                [self doInstallSkeyCert];
                break;
            case COMMAND_CLIENT_PIN_SET:
            case COMMAND_CLIENT_PIN_CHANGE:
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                [self doClientPin];
                break;
            default:
                // エラーメッセージを表示
                [ToolPopupWindow critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil];
                [[self delegate] hidCommandDidProcess:command result:false message:nil];
                break;
        }
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        if (![[self toolHIDHelper] isDeviceConnected]) {
            [ToolPopupWindow critical:MSG_CMDTST_PROMPT_USB_PORT_SET informativeText:nil];
            return false;
        }
        return true;
    }

#pragma mark - Call back from ToolHIDHelper

    - (void)hidHelperDidReceive:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd {
        // レスポンスタイムアウト監視を停止
        [self cancelResponseTimeoutMonitor];
        // コマンドに応じ、以下の処理に分岐
        switch (cmd) {
            case HID_CMD_CTAPHID_INIT:
                [self doResponseCtapHidInit:message];
                break;
            case HID_CMD_ERASE_SKEY_CERT:
            case HID_CMD_INSTALL_SKEY_CERT:
                [self doResponseMaintenanceCommand:message];
                break;
            case HID_CMD_CTAPHID_CBOR:
                [self doResponseCtapHidCbor:message CID:cid CMD:cmd];
                break;
            default:
                break;
        }
    }

    - (bool)checkStatusCode:(NSData *)responseMessage {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        char *requestBytes = (char *)[responseMessage bytes];
        switch (requestBytes[0]) {
            case CTAP1_ERR_SUCCESS:
                return true;
            case CTAP2_ERR_PIN_INVALID:
            case CTAP2_ERR_PIN_AUTH_INVALID:
                [self displayMessage:MSG_CTAP2_ERR_PIN_INVALID];
                break;
            case CTAP2_ERR_PIN_BLOCKED:
                [self displayMessage:MSG_CTAP2_ERR_PIN_BLOCKED];
                break;
            case CTAP2_ERR_PIN_AUTH_BLOCKED:
                [self displayMessage:MSG_CTAP2_ERR_PIN_AUTH_BLOCKED];
                break;
            case CTAP2_ERR_PIN_NOT_SET:
                [self displayMessage:MSG_CTAP2_ERR_PIN_NOT_SET];
                break;
            default:
                break;
        }
        return false;
    }

#pragma mark - Interface for SetPinParamWindow

    - (void)setPinParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ダイアログをモーダルで表示
        [[self toolClientPINCommand] setPinParamWindowWillOpen:sender
                                                  parentWindow:parentWindow toolCommand:self];
    }

    - (void)setPinParamWindowDidClose {
        // AppDelegateに制御を戻す
        [[self delegate] hidCommandDidProcess:COMMAND_NONE result:true message:nil];
    }

#pragma mark - Interface for PinCodeParamWindow

    - (void)pinCodeParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ダイアログをモーダルで表示
        [[self toolCTAP2HealthCheckCommand] pinCodeParamWindowWillOpen:sender
                                            parentWindow:parentWindow toolCommand:self];
    }

    - (void)pinCodeParamWindowDidClose {
        // AppDelegateに制御を戻す
        [[self delegate] hidCommandDidProcess:COMMAND_NONE result:true message:nil];
    }

@end
