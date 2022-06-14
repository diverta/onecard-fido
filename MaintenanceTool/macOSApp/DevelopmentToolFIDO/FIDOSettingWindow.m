//
//  FIDOSettingWindow.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/08.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "FIDOSettingWindow.h"
#import "ToolPopupWindow.h"
#import "UtilityCommand.h"

@interface FIDOSettingWindow ()

    // 画面項目
    @property (assign) IBOutlet NSButton                   *buttonFIDOAttestation;
    @property (assign) IBOutlet NSButton                   *buttonReset;
    @property (assign) IBOutlet NSButton                   *buttonCancel;
    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // 実行するコマンドを保持
    @property (nonatomic) Command                           command;

@end

@implementation FIDOSettingWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)setParentWindowRef:(id)ref {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
    }

    - (IBAction)buttonFIDOAttestationDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK withCommand:COMMAND_FIDO_ATTESTATION];
    }

    - (IBAction)buttonResetDidPress:(id)sender {
        // 処理開始前に確認
        [[ToolPopupWindow defaultWindow] criticalPrompt:MSG_ERASE_SKEY_CERT informativeText:MSG_PROMPT_ERASE_SKEY_CERT
                                             withObject:self forSelector:@selector(resumeFIDOAttestationReset) parentWindow:[self window]];
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

    - (void)resumeFIDOAttestationReset {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK withCommand:COMMAND_FIDO_ATTESTATION_RESET];
    }

#pragma mark - Interface for parameters

    - (Command)commandToPerform {
        // 実行コマンドを戻す
        return [self command];
    }

@end
