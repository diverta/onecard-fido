//
//  FIDOSettingWindow.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/08.
//
#import "AppDefine.h"
#import "UtilityCommand.h"
#import "FIDOSettingWindow.h"

@interface FIDOSettingWindow ()

    // 画面項目
    @property (assign) IBOutlet NSButton                   *buttonFIDOAttestation;
    @property (assign) IBOutlet NSButton                   *buttonReset;
    @property (assign) IBOutlet NSButton                   *buttonCancel;
    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) UtilityCommand             *utilityCommand;
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

    - (void)setCommandRef:(id)ref {
        // コマンドクラスの参照を保持
        [self setUtilityCommand:(UtilityCommand *)ref];
    }

    - (IBAction)buttonFIDOAttestationDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK withCommand:COMMAND_VIEW_APP_VERSION];
    }

    - (IBAction)buttonResetDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK withCommand:COMMAND_VIEW_LOG_FILE];
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

#pragma mark - Interface for parameters

    - (Command)commandToPerform {
        // 実行コマンドを戻す
        return [self command];
    }

@end
