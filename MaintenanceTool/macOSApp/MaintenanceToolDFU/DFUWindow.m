//
//  ToolDFUWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/18.
//
#import "AppCommonMessage.h"
#import "DFUCommand.h"
#import "DFUWindow.h"
#import "ToolPopupWindow.h"

@interface DFUWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) DFUCommand             *dfuCommandRef;
    // DFU処理のパラメーターを保持
    @property (nonatomic) DFUCommandParameter          *commandParameterRef;

@end

@implementation DFUWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
        // コマンドクラスの参照を保持
        [self setDfuCommandRef:(DFUCommand *)commandRef];
        // DFU処理のパラメーターを保持
        [self setCommandParameterRef:parameterRef];
        // パラメーターを初期化
        [[self commandParameterRef] setTransportType:TRANSPORT_NONE];
    }

    - (IBAction)buttonUSBDFUDidPress:(id)sender {
        // USB接続チェック
        if ([[self dfuCommandRef] isUSBHIDConnected]) {
            // パラメーターを設定
            [[self commandParameterRef] setTransportType:TRANSPORT_HID];
            // このウィンドウを終了
            [self terminateWindow:NSModalResponseOK];
        }
    }

    - (IBAction)buttonBLEDFUDidPress:(id)sender {
        // DFU処理を開始するかどうかのプロンプトを表示
        [[ToolPopupWindow defaultWindow] informationalPrompt:MSG_PROMPT_START_BLE_DFU_PROCESS informativeText:MSG_COMMENT_START_BLE_DFU_PROCESS
                                                  withObject:self forSelector:@selector(bleDfuCommandPromptDone) parentWindow:[self window]];
    }

    - (void)bleDfuCommandPromptDone {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // パラメーターを設定
        [[self commandParameterRef] setTransportType:TRANSPORT_BLE];
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
