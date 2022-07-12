//
//  HcheckWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/12.
//
#import "AppDefine.h"
#import "HcheckCommand.h"
#import "HcheckWindow.h"

@interface HcheckWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) id                          hcheckCommandRef;
    // 実行するコマンドを保持
    @property (nonatomic) Command                           command;

@end

@implementation HcheckWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)setParentWindowRef:(id)ref {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
    }

    - (void)setCommandRef:(id)ref {
        // コマンドクラスの参照を保持
        [self setHcheckCommandRef:ref];
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
