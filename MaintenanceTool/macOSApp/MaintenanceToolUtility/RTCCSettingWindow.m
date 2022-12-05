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
    }

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
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

@end
