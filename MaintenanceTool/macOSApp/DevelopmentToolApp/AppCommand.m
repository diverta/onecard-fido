//
//  AppCommand.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/14.
//
#import "AppCommand.h"
#import "AppCommonMessage.h"
#import "ToolLogFile.h"
#import "ToolPopupWindow.h"

@interface AppCommand ()

@end

@implementation AppCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
        }
        return self;
    }

    - (void)notifyCommandStarted:(NSString *)commandName {
        // ボタンを不活性化
        [[self delegate] enableButtonsOfMainUI:false];
        if (commandName) {
            // コマンド開始メッセージを画面表示し、ログファイルにも出力
            NSString *startMsg = [NSString stringWithFormat:MSG_FORMAT_START_MESSAGE, commandName];
            [[self delegate] notifyMessageToMainUI:startMsg];
            [[ToolLogFile defaultLogger] info:startMsg];
        }
    }

    - (void)notifyCommandTerminated:(NSString *)commandName message:(NSString *)message success:(bool)success fromWindow:(NSWindow *)window {
        if (success == false) {
            // 処理失敗時は、引数に格納されたエラーメッセージを画面出力
            [[self delegate] notifyMessageToMainUI:message];
        }
        if (commandName == nil) {
            // ボタンを活性化
            [[self delegate] enableButtonsOfMainUI:true];
            return;
        }
        // テキストエリアとポップアップの両方に表示させる処理終了メッセージを作成
        NSString *str = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE,
                         commandName, success ? MSG_SUCCESS : MSG_FAILURE];
        // メッセージを画面のテキストエリアに表示
        [[self delegate] notifyMessageToMainUI:str];
        // メッセージをログファイルに出力してから、ポップアップを表示-->ボタンを活性化
        if (success) {
            [[ToolLogFile defaultLogger] info:str];
            [[ToolPopupWindow defaultWindow] informational:str informativeText:nil withObject:self
                                               forSelector:@selector(displayCommandResultDone) parentWindow:window];
        } else {
            [[ToolLogFile defaultLogger] error:str];
            [[ToolPopupWindow defaultWindow] critical:str informativeText:nil withObject:self
                                          forSelector:@selector(displayCommandResultDone) parentWindow:window];
        }
    }

    - (void)displayCommandResultDone {
        // ボタンを活性化
        [[self delegate] enableButtonsOfMainUI:true];
    }

@end
