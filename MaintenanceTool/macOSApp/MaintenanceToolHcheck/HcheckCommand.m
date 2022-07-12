//
//  HcheckCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/12.
//
#import "AppHIDCommand.h"
#import "HcheckCommand.h"
#import "HcheckWindow.h"
#import "ToolLogFile.h"

@implementation HcheckCommandParameter

@end

@interface HcheckCommand () <AppHIDCommandDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) HcheckWindow                 *hcheckWindow;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;
    // ヘルスチェック処理のパラメーターを保持
    @property (nonatomic) HcheckCommandParameter       *commandParameter;

@end

@implementation HcheckCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // 画面のインスタンスを生成
            [self setHcheckWindow:[[HcheckWindow alloc] initWithWindowNibName:@"HcheckWindow"]];
            // ヘルパークラスのインスタンスを生成
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
            [self setCommandParameter:[[HcheckCommandParameter alloc] init]];
        }
        return self;
    }

    - (void)hcheckWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self hcheckWindow] setParentWindowRef:parentWindow withCommandRef:self withParameterRef:[self commandParameter]];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self hcheckWindow] window];
        HcheckCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf hcheckWindowDidClose:self modalResponse:response];
        }];
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self appHIDCommand] checkUSBHIDConnection];
    }

#pragma mark - Perform functions

    - (void)hcheckWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self hcheckWindow] close];
        // TODO: 設定されたパラメーターを使用し、ヘルスチェック処理を実行する
        [[ToolLogFile defaultLogger] debugWithFormat:@"command:%d, pin:%@", [[self commandParameter] command], [[self commandParameter] pin]];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時でアプリケーションに制御を戻す
        if (success == false) {
            [self notifyCommandTerminated:[self commandName] message:errorMessage success:success fromWindow:[self parentWindow]];
            return;
        }
        // 実行コマンドにより処理分岐
        switch (command) {
            default:
                // メイン画面に制御を戻す
                [self notifyCommandTerminated:[self commandName] message:nil success:success fromWindow:[self parentWindow]];
                break;
        }
    }

@end
