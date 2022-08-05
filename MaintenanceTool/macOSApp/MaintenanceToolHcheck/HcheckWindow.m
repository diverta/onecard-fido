//
//  HcheckWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/12.
//
#import "AppDefine.h"
#import "HcheckCommand.h"
#import "HcheckPinWindow.h"
#import "HcheckWindow.h"
#import "ToolCommonFunc.h"

@interface HcheckWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) HcheckPinWindow                  *hcheckPinWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) id                          hcheckCommandRef;
    // ヘルスチェック処理のパラメーターを保持
    @property (nonatomic) HcheckCommandParameter           *commandParameterRef;

@end

@implementation HcheckWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        if ([self hcheckPinWindow] == nil) {
            // 画面のインスタンスを生成
            [self setHcheckPinWindow:[[HcheckPinWindow alloc] initWithWindowNibName:@"HcheckPinWindow"]];
        }
    }

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
        // コマンドクラスの参照を保持
        [self setHcheckCommandRef:commandRef];
        // ヘルスチェック処理のパラメーターを保持
        [self setCommandParameterRef:parameterRef];
        // パラメーターを初期化
        [[self commandParameterRef] setCommand:COMMAND_NONE];
        [[self commandParameterRef] setPin:@""];
    }

    - (IBAction)buttonBLECtap2HealthCheckDidPress:(id)sender {
        // PIN番号入力画面を開く
        [[self commandParameterRef] setCommand:COMMAND_BLE_CTAP2_HCHECK];
        [self hcheckPinWindowWillOpen];
    }

    - (IBAction)buttonBLEU2FHealthCheckDidPress:(id)sender {
        [[self commandParameterRef] setCommand:COMMAND_BLE_U2F_HCHECK];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonBLEPingTestDidPress:(id)sender {
        [[self commandParameterRef] setCommand:COMMAND_TEST_BLE_PING];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonHIDCtap2HealthCheckDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // PIN番号入力画面を開く
        [[self commandParameterRef] setCommand:COMMAND_HID_CTAP2_HCHECK];
        [self hcheckPinWindowWillOpen];
    }

    - (IBAction)buttonHIDU2FHealthCheckDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        [[self commandParameterRef] setCommand:COMMAND_HID_U2F_HCHECK];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonHIDPingTestDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        [[self commandParameterRef] setCommand:COMMAND_TEST_CTAPHID_PING];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合は処理中止
        HcheckCommand *command = (HcheckCommand *)[self hcheckCommandRef];
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:[self window] connected:[command isUSBHIDConnected]];
    }

#pragma mark - Interface for PIN entry window

    - (void)hcheckPinWindowWillOpen {
        // 画面に親画面参照をセット
        [[self hcheckPinWindow] setParentWindowRef:[self window] withCommandRef:[self hcheckCommandRef] withParameterRef:[self commandParameterRef]];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self hcheckPinWindow] window];
        HcheckWindow * __weak weakSelf = self;
        [[self window] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf hcheckPinWindowDidClose:self modalResponse:response];
        }];
    }

    - (void)hcheckPinWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // PIN入力画面を閉じる
        [[self hcheckPinWindow] close];
        if (modalResponse == NSModalResponseOK) {
            // この画面も閉じる
            [self terminateWindow:NSModalResponseOK];
        }
    }

@end
