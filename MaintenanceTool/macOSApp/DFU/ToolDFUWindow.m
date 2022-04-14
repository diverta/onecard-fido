//
//  ToolDFUWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/11/22.
//
#import "ToolDFUCommand.h"
#import "ToolDFUWindow.h"
#import "ToolCommonMessage.h"
#import "ToolPopupWindow.h"

@interface ToolDFUWindow ()

    // 画面項目
    @property (assign) IBOutlet NSButton                   *buttonUSBDFU;
    @property (assign) IBOutlet NSButton                   *buttonBLEDFU;
    @property (assign) IBOutlet NSButton                   *buttonCancel;
    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) ToolDFUCommand             *toolDFUCommand;
    // 実行するコマンドを保持
    @property (nonatomic) Command                           command;

@end

@implementation ToolDFUWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)setParentWindowRef:(id)ref {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
    }

    - (void)setCommandRef:(id)ref {
        // コマンドクラスの参照を保持
        [self setToolDFUCommand:(ToolDFUCommand *)ref];
    }

    - (IBAction)buttonUSBDFUDidPress:(id)sender {
        // USB接続チェック
        if ([[self toolDFUCommand] checkUSBHIDConnection]) {
            // このウィンドウを終了
            [self terminateWindow:NSModalResponseOK withCommand:COMMAND_USB_DFU];
        } else {
            // エラーメッセージをポップアップ表示
            [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_PROMPT_USB_PORT_SET informativeText:nil withObject:nil forSelector:nil];
        }
    }

    - (IBAction)buttonBLEDFUDidPress:(id)sender {
        // DFU処理を開始するかどうかのプロンプトを表示
        [[ToolPopupWindow defaultWindow] informationalPrompt:MSG_PROMPT_START_BLE_DFU_PROCESS informativeText:MSG_COMMENT_START_BLE_DFU_PROCESS
                                                  withObject:self forSelector:@selector(bleDfuCommandPromptDone) parentWindow:[self window]];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseCancel withCommand:COMMAND_NONE];
    }

    - (void)terminateWindow:(NSModalResponse)response withCommand:(Command)command {
        // 実行コマンドを保持
        [self setCommand:command];
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

    - (void)bleDfuCommandPromptDone {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK withCommand:COMMAND_BLE_DFU];
    }

#pragma mark - Interface for parameters

    - (Command)commandToPerform {
        // 実行コマンドを戻す
        return [self command];
    }

@end
