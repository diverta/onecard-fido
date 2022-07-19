//
//  FIDOSettingCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/18.
//
#import "FIDOSettingCommand.h"
#import "FIDOSettingWindow.h"

@implementation FIDOSettingCommandParameter

@end

@interface FIDOSettingCommand ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) FIDOSettingWindow            *fidoSettingWindow;
    // PIN番号管理処理のパラメーターを保持
    @property (nonatomic) FIDOSettingCommandParameter  *commandParameter;

@end

@implementation FIDOSettingCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // 画面のインスタンスを生成
            [self setFidoSettingWindow:[[FIDOSettingWindow alloc] initWithWindowNibName:@"FIDOSettingWindow"]];
            // ヘルパークラスのインスタンスを生成
            [self setCommandParameter:[[FIDOSettingCommandParameter alloc] init]];
        }
        return self;
    }

    - (void)fidoSettingWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self fidoSettingWindow] setParentWindowRef:parentWindow withCommandRef:self withParameterRef:[self commandParameter]];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self fidoSettingWindow] window];
        FIDOSettingCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf fidoSettingWindowDidClose:self modalResponse:response];
        }];
    }

    - (bool)isUSBHIDConnected {
        // TODO: USBポートに接続されていない場合はfalse
        return false;
    }

#pragma mark - Perform functions

    - (void)fidoSettingWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self fidoSettingWindow] close];
    }

@end
