//
//  HcheckCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/12.
//
#import "AppCommonMessage.h"
#import "CTAP2HcheckCommand.h"
#import "FIDODefines.h"
#import "HcheckCommand.h"
#import "HcheckWindow.h"
#import "ToolCommon.h"
#import "ToolLogFile.h"
#import "U2FHcheckCommand.h"

@implementation HcheckCommandParameter

@end

@interface HcheckCommand () <CTAP2HcheckCommandDelegate, U2FHcheckCommandDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) HcheckWindow                 *hcheckWindow;
    // 下位クラスの参照を保持
    @property (nonatomic) CTAP2HcheckCommand           *ctap2HcheckCommand;
    @property (nonatomic) U2FHcheckCommand             *u2fHcheckCommand;
    // ヘルスチェック処理のパラメーターを保持
    @property (nonatomic) HcheckCommandParameter       *commandParameter;
    // PINGデータを保持
    @property (nonatomic) NSData                       *pingData;

@end

@implementation HcheckCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // 画面のインスタンスを生成
            [self setHcheckWindow:[[HcheckWindow alloc] initWithWindowNibName:@"HcheckWindow"]];
            // ヘルパークラスのインスタンスを生成
            [self setCommandParameter:[[HcheckCommandParameter alloc] init]];
            [self setCtap2HcheckCommand:[[CTAP2HcheckCommand alloc] initWithDelegate:self]];
            [self setU2fHcheckCommand:[[U2FHcheckCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (void)hcheckWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self hcheckWindow] setParentWindowRef:parentWindow withCommandRef:self withParameterRef:[self commandParameter]];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self hcheckWindow] window];
        HcheckCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf hcheckWindowDidClose:self modalResponse:response];
        }];
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self u2fHcheckCommand] isUSBHIDConnected];
    }

#pragma mark - Perform functions

    - (void)hcheckWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self hcheckWindow] close];
        // TODO: 設定されたパラメーターを使用し、ヘルスチェック処理を実行する
        [[ToolLogFile defaultLogger] debugWithFormat:@"command:%d, pin:%@", [[self commandParameter] command], [[self commandParameter] pin]];
        // 実行コマンドにより処理分岐
        switch ([[self commandParameter] command]) {
            case COMMAND_HID_CTAP2_HCHECK:
                [self notifyCommandStartedWithCommandName:PROCESS_NAME_HID_CTAP2_HEALTHCHECK];
                [[self ctap2HcheckCommand] doRequestHidCtap2HealthCheck:[self commandParameter]];
                break;
            case COMMAND_HID_U2F_HCHECK:
                [self notifyCommandStartedWithCommandName:PROCESS_NAME_HID_U2F_HEALTHCHECK];
                [[self u2fHcheckCommand] doRequestHidU2fHealthCheck];
                break;
            case COMMAND_TEST_CTAPHID_PING:
                [self notifyCommandStartedWithCommandName:PROCESS_NAME_TEST_CTAPHID_PING];
                [[self u2fHcheckCommand] doRequestHidPingTest];
                break;
            case COMMAND_BLE_U2F_HCHECK:
                [self notifyCommandStartedWithCommandName:PROCESS_NAME_BLE_U2F_HEALTHCHECK];
                [[self u2fHcheckCommand] doRequestBleU2fHealthCheck];
                break;
            case COMMAND_TEST_BLE_PING:
                [self notifyCommandStartedWithCommandName:PROCESS_NAME_TEST_BLE_PING];
                [[self u2fHcheckCommand] doRequestBlePingTest];
                break;
            default:
                // メイン画面に制御を戻す
                break;
        }
    }

    - (void)notifyCommandStartedWithCommandName:(NSString *)commandName {
        // コマンド開始メッセージを画面表示
        [self setCommandName:commandName];
        [self notifyCommandStarted:[self commandName]];
    }

#pragma mark - Call back from CTAP2HcheckCommand

    - (void)doResponseCtap2HealthCheck:(bool)success message:(NSString *)message {
        // メイン画面に制御を戻す
        [self notifyCommandTerminated:[self commandName] message:message success:success fromWindow:[self parentWindow]];
    }

#pragma mark - Call back from U2FHcheckCommand

    - (void)doResponseU2fHealthCheck:(bool)success message:(NSString *)message {
        // メイン画面に制御を戻す
        [self notifyCommandTerminated:[self commandName] message:message success:success fromWindow:[self parentWindow]];
    }

    - (void)notifyMessage:(NSString *)message {
        // メイン画面にテキストを表示
        [[self delegate] notifyMessageToMainUI:message];
    }

@end
