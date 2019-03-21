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
#import "ToolPopupWindow.h"

// HIDコマンドバイト
#define HID_CMD_CTAPHID_INIT        0x86
#define HID_CMD_ERASE_SKEY_CERT     0xC0
#define HID_CMD_INSTALL_SKEY_CERT   0xC1

@interface ToolHIDCommand () <ToolHIDHelperDelegate>

    @property (nonatomic) ToolHIDHelper      *toolHIDHelper;
    @property (nonatomic) ToolInstallCommand *toolInstallCommand;

    @property (nonatomic) Command   command;
    @property (nonatomic) NSString *skeyFilePath;
    @property (nonatomic) NSString *certFilePath;

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

    - (void)doRequest:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd {
        // HIDデバイスにリクエストを送信
        [[self toolHIDHelper] hidHelperWillSend:message CID:cid CMD:cmd];
        // レスポンスタイムアウトを監視
        [self startResponseTimeoutMonitor];
    }

    - (void)doResponseToAppDelegate:(bool)result {
        // テキストエリアとポップアップの両方に表示させる処理終了メッセージを作成
        NSString *message = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE,
                         [ToolCommon processNameOfCommand:[self command]],
                         result? MSG_SUCCESS:MSG_FAILURE];
        // AppDelegateに制御を戻す
        [[self delegate] hidCommandDidProcess:[self command] result:result message:message];
    }

    - (void)doTestCtapHidInit {
        // CTAPHID_INITコマンドをテスト実行する
        NSData *message = [[NSData alloc] initWithBytes:nonceBytes length:sizeof(nonceBytes)];
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [self doRequest:message CID:cid CMD:HID_CMD_CTAPHID_INIT];
    }

    - (void)doResponseCtapHidInit:(NSData *)message {
        // レスポンスメッセージの先頭８バイトと、リクエスト時のnonceが一致しているか確認
        char *requestBytes = (char *)[message bytes];
        bool result = (memcmp(requestBytes, nonceBytes, sizeof(nonceBytes)) == 0);
        // AppDelegateに制御を戻す
        [self doResponseToAppDelegate:result];
    }

    - (void)doEraseSkeyCert {
        // メッセージを編集し、コマンド 0xC0 を実行
        NSData *message = [[self toolInstallCommand] generateEraseSkeyCertMessage:[self command]];
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [self doRequest:message CID:cid CMD:HID_CMD_ERASE_SKEY_CERT];
    }

    - (void)doInstallSkeyCert {
        // メッセージを編集し、コマンド 0xC1 を実行
        NSData *message = [[self toolInstallCommand] generateInstallSkeyCertMessage:[self command]
                            skeyFilePath:[self skeyFilePath] certFilePath:[self certFilePath]];
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [self doRequest:message CID:cid CMD:HID_CMD_INSTALL_SKEY_CERT];
    }

    - (void)doResponseMaintenanceCommand:(NSData *)message {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        char *requestBytes = (char *)[message bytes];
        bool result = (requestBytes[0] == 0x00);
        // AppDelegateに制御を戻す
        [self doResponseToAppDelegate:result];
    }

    - (void)hidHelperWillProcess:(Command)command {
        // USBポートに接続されていない場合は終了
        if (![[self toolHIDHelper] isDeviceConnected]) {
            [ToolPopupWindow critical:MSG_CMDTST_PROMPT_USB_PORT_SET informativeText:nil];
            [[self delegate] hidCommandDidProcess:command result:false message:nil];
            return;
        }
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
            case COMMAND_INSTALL_SKEY:
                [self doInstallSkeyCert];
                break;
            default:
                // エラーメッセージを表示
                [ToolPopupWindow critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil];
                [[self delegate] hidCommandDidProcess:command result:false message:nil];
                break;
        }
    }

#pragma mark - Call back from ToolHIDHelper

    - (void)hidHelperDidReceive:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd {
        // レスポンスタイムアウト監視を停止
        [self cancelResponseTimeoutMonitor];
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_TEST_CTAPHID_INIT:
                [self doResponseCtapHidInit:message];
                break;
            case COMMAND_ERASE_SKEY_CERT:
            case COMMAND_INSTALL_SKEY:
                [self doResponseMaintenanceCommand:message];
                break;
            default:
                break;
        }
    }

@end
