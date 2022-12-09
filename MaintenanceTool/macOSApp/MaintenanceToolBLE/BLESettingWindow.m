//
//  BLESettingWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/19.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "BLESettingCommand.h"
#import "BLESettingWindow.h"
#import "ToolCommonFunc.h"
#import "ToolPopupWindow.h"

@interface BLESettingWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) id                          bleSettingCommandRef;
    // ヘルスチェック処理のパラメーターを保持
    @property (nonatomic) BLESettingCommandParameter       *commandParameterRef;

@end

@implementation BLESettingWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
        // コマンドクラスの参照を保持
        [self setBleSettingCommandRef:commandRef];
        // ヘルスチェック処理のパラメーターを保持
        [self setCommandParameterRef:parameterRef];
        // パラメーターを初期化
        [[self commandParameterRef] setCommand:COMMAND_NONE];
    }

    - (IBAction)buttonPairingDidPress:(id)sender {
        // 実行コマンドを設定して画面を閉じる
        [[self commandParameterRef] setCommand:COMMAND_PAIRING];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonUnpairingRequestDidPress:(id)sender {
        // 実行コマンドを設定して画面を閉じる
        [[self commandParameterRef] setCommand:COMMAND_UNPAIRING_REQUEST];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonUnpairingDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 処理続行確認ダイアログを開く
        [[ToolPopupWindow defaultWindow] criticalPrompt:MSG_ERASE_BONDS informativeText:MSG_PROMPT_ERASE_BONDS
                                             withObject:self forSelector:@selector(unpairingCommandPromptDone) parentWindow:[self window]];
    }

    - (void)unpairingCommandPromptDone {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // 実行コマンドを設定して画面を閉じる
        [[self commandParameterRef] setCommand:COMMAND_ERASE_BONDS];
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
        BLESettingCommand *command = (BLESettingCommand *)[self bleSettingCommandRef];
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:[self window] connected:[command isUSBHIDConnected]];
    }

@end
