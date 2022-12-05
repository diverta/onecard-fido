//
//  RTCCSettingWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/18.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "RTCCSettingCommand.h"
#import "RTCCSettingWindow.h"
#import "ToolCommonFunc.h"
#import "ToolPopupWindow.h"

@interface RTCCSettingWindow ()

    // 画面項目を保持
    @property (assign) IBOutlet NSButton               *buttonTransportUSB;
    @property (assign) IBOutlet NSButton               *buttonTransportBLE;
    @property (assign) IBOutlet NSTextField            *LabelToolTimestamp;
    @property (assign) IBOutlet NSTextField            *LabelDeviceTimestamp;
    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic) RTCCSettingCommand           *commandRef;
    // トランスポート種別を保持
    @property (nonatomic) TransportType                 transportType;

@end

@implementation RTCCSettingWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        [self setCommandRef:[[RTCCSettingCommand alloc] init]];
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // ラジオボタン「USB経由」を選択状態にする
        [[self buttonTransportUSB] setState:NSControlStateValueOn];
        [self setTransportType:TRANSPORT_HID];
        // 時刻表示ラベルをブランクにする
        [[self LabelToolTimestamp] setStringValue:@""];
        [[self LabelDeviceTimestamp] setStringValue:@""];
    }

    - (IBAction)buttonTransportSelected:(id)sender {
        // トランスポート種別を設定
        if (sender == [self buttonTransportUSB]) {
            [self setTransportType:TRANSPORT_HID];
        }
        if (sender == [self buttonTransportBLE]) {
            [self setTransportType:TRANSPORT_BLE];
        }
    }

    - (IBAction)buttonGetTimestampDidPress:(id)sender {
        // USB接続チェック
        if ([self transportType] == TRANSPORT_HID) {
            if ([self checkUSBHIDConnection] == false) {
                return;
            }
        }
        // 現在時刻参照を実行
        [[self commandRef] commandWillPerform:COMMAND_RTCC_GET_TIMESTAMP withTransportType:[self transportType]];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合は処理中止
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:[self window] connected:[[self commandRef] isUSBHIDConnected]];
    }

    - (IBAction)buttonSetTimestampDidPress:(id)sender {
        // USB接続チェック
        if ([self transportType] == TRANSPORT_HID) {
            if ([self checkUSBHIDConnection] == false) {
                return;
            }
        }
        // 処理を実行するかどうかのプロンプトを表示
        [[ToolPopupWindow defaultWindow] informationalPrompt:MSG_PROMPT_RTCC_SET_TIMESTAMP informativeText:MSG_COMMENT_RTCC_SET_TIMESTAMP
                                                  withObject:self forSelector:@selector(SetTimestampCommandPromptDone) parentWindow:[self window]];
    }

    - (void)SetTimestampCommandPromptDone {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // 現在時刻設定を実行
        [[self commandRef] commandWillPerform:COMMAND_RTCC_SET_TIMESTAMP withTransportType:[self transportType]];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

#pragma mark - For RTCCSettingWindow open/close

    - (bool)windowWillOpenWithParentWindow:(NSWindow *)parent {
        // 親画面の参照を保持
        [self setParentWindow:parent];
        // すでにダイアログが開いている場合は終了
        if ([[[self parentWindow] sheets] count] > 0) {
            return false;
        }
        // すでにダイアログがロード済みの場合は、画面項目を再度初期化
        if ([self isWindowLoaded]) {
            [self initFieldValue];
        }
        // ダイアログをモーダルで表示
        NSWindow *dialog = [self window];
        RTCCSettingWindow * __weak weakSelf = self;
        [[self parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf windowDidCloseWithModalResponse:response];
        }];
        return true;
    }

    - (void)windowDidCloseWithModalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [self close];
    }

@end
