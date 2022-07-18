//
//  FIDOSettingWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/18.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "FIDOSettingCommand.h"
#import "FIDOSettingWindow.h"
#import "ToolCommonFunc.h"
#import "ToolPopupWindow.h"

@interface FIDOSettingWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) id                          fidoSettingCommandRef;
    // ヘルスチェック処理のパラメーターを保持
    @property (nonatomic) FIDOSettingCommandParameter      *commandParameterRef;

@end

@implementation FIDOSettingWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
        // コマンドクラスの参照を保持
        [self setFidoSettingCommandRef:commandRef];
        // ヘルスチェック処理のパラメーターを保持
        [self setCommandParameterRef:parameterRef];
        // パラメーターを初期化
        [[self commandParameterRef] setCommand:COMMAND_NONE];
        [[self commandParameterRef] setPinNew:@""];
        [[self commandParameterRef] setPinOld:@""];
    }

    - (IBAction)buttonSetPinParamDidPress:(id)sender {
        // TODO: PIN番号入力画面を開く
        [[self commandParameterRef] setCommand:COMMAND_NONE];
        [[self commandParameterRef] setPinNew:@""];
        [[self commandParameterRef] setPinOld:@""];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonResetDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 処理続行確認ダイアログを開く
        [[ToolPopupWindow defaultWindow] criticalPrompt:MSG_CLEAR_PIN_CODE informativeText:MSG_PROMPT_CLEAR_PIN_CODE
                                             withObject:self forSelector:@selector(clearPinCommandPromptDone) parentWindow:[self window]];
    }

    - (void)clearPinCommandPromptDone {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // PINコード、実行コマンドを設定して画面を閉じる
        [[self commandParameterRef] setCommand:COMMAND_AUTH_RESET];
        [[self commandParameterRef] setPinNew:@""];
        [[self commandParameterRef] setPinOld:@""];
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
        FIDOSettingCommand *command = (FIDOSettingCommand *)[self fidoSettingCommandRef];
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:[self window] connected:[command isUSBHIDConnected]];
    }

@end
