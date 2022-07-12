//
//  HcheckCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/12.
//
#import "AppHIDCommand.h"
#import "HcheckCommand.h"
#import "HcheckWindow.h"

@interface HcheckCommand () <AppHIDCommandDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) HcheckWindow                 *hcheckWindow;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;

@end

@implementation HcheckCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // 画面のインスタンスを生成
            [self setHcheckWindow:[[HcheckWindow alloc] initWithWindowNibName:@"HcheckWindow"]];
            // ヘルパークラスのインスタンスを生成
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (void)hcheckWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self hcheckWindow] setParentWindowRef:parentWindow];
        [[self hcheckWindow] setCommandRef:self];
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
        // 実行コマンドにより処理分岐
        switch ([[self hcheckWindow] commandToPerform]) {
            default:
                // メイン画面に制御を戻す
                break;
        }
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
