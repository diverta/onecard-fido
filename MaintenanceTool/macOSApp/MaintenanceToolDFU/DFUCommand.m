//
//  DFUCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/20.
//
#import "AppCommonMessage.h"
#import "DFUCommand.h"
#import "ToolBLEDFUCommand.h"

@interface DFUCommand () <ToolBLEDFUCommandDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 下位クラスの参照を保持
    @property (nonatomic) ToolBLEDFUCommand            *toolBLEDFUCommand;

@end

@implementation DFUCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // ヘルパークラスのインスタンスを生成
            [self setToolBLEDFUCommand:[[ToolBLEDFUCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (void)bleDfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ファームウェア更新処理を実行
        [[self toolBLEDFUCommand] bleDfuProcessWillStart:sender parentWindow:parentWindow];
    }

#pragma mark - Perform functions

    - (void)notifyCommandStartedWithCommandName:(NSString *)commandName {
        // コマンド開始メッセージを画面表示
        [self setCommandName:commandName];
        [super notifyCommandStarted:[self commandName]];
    }

#pragma mark - Call back from ToolBLEDFUCommand

    - (void)notifyCommandStarted:(Command)command {
        [self notifyCommandStartedWithCommandName:PROCESS_NAME_BLE_DFU];
    }

    - (void)notifyCommandTerminated:(Command)command success:(bool)success message:(NSString *)message {
        // メイン画面に制御を戻す
        [self notifyCommandTerminated:[self commandName] message:message success:success fromWindow:[self parentWindow]];
}

    - (void)notifyMessage:(NSString *)message {
        [[self delegate] notifyMessageToMainUI:message];
    }

@end
