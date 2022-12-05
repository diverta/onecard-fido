//
//  RTCCSettingWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/18.
//
#import "AppCommonMessage.h"
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

@end

@implementation RTCCSettingWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // ラジオボタン「USB経由」を選択状態にする
        [[self buttonTransportUSB] setState:NSControlStateValueOn];
        // 時刻表示ラベルをブランクにする
        [[self LabelToolTimestamp] setStringValue:@""];
        [[self LabelDeviceTimestamp] setStringValue:@""];
    }

    - (IBAction)buttonTransportSelected:(id)sender {
        // トランスポート種別を設定
        if (sender == [self buttonTransportUSB]) {
        }
        if (sender == [self buttonTransportBLE]) {
        }
    }

    - (IBAction)buttonGetTimestampDidPress:(id)sender {
        // USB接続チェック
        if ([self checkUSBHIDConnection]) {
            // このウィンドウを終了
            [self terminateWindow:NSModalResponseOK];
        }
    }

    - (bool)checkUSBHIDConnection {
        // TODO: 仮の実装です。
        return true;
    }

    - (IBAction)buttonSetTimestampDidPress:(id)sender {
        // USB接続チェック
        if ([self checkUSBHIDConnection]) {
            // 処理を実行するかどうかのプロンプトを表示
            [[ToolPopupWindow defaultWindow] informationalPrompt:MSG_PROMPT_RTCC_SET_TIMESTAMP informativeText:MSG_COMMENT_RTCC_SET_TIMESTAMP
                                                      withObject:self forSelector:@selector(SetTimestampCommandPromptDone) parentWindow:[self window]];
        }
    }

    - (void)SetTimestampCommandPromptDone {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // このウィンドウを終了
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

#pragma mark - For RTCCSettingWindow open/close

    - (bool)windowWillOpenWithCommandRef:(id)ref parentWindow:(NSWindow *)parent {
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
            [weakSelf windowDidCloseWithSender:ref modalResponse:response];
        }];
        return true;
    }

    - (void)windowDidCloseWithSender:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [self close];
    }

@end
