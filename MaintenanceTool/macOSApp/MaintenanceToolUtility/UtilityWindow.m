//
//  UtilityWindow.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/07.
//
#import "AppDefine.h"
#import "UtilityCommand.h"
#import "UtilityWindow.h"

@interface UtilityWindow ()

    // 画面項目
    @property (assign) IBOutlet NSButton                   *buttonToolVersionInfo;
    @property (assign) IBOutlet NSButton                   *buttonViewLogFile;
    @property (assign) IBOutlet NSButton                   *buttonCancel;
    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) UtilityCommand             *utilityCommand;
    // 実行するコマンドを保持
    @property (nonatomic) Command                           command;

@end

@implementation UtilityWindow

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

    - (IBAction)buttonToolVersionInfoDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK withCommand:COMMAND_VIEW_APP_VERSION];
    }

    - (IBAction)buttonViewLogFileDidPress:(id)sender {
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
