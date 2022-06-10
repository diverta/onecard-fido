//
//  FIDOSettingCommand.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/08.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "FIDOSettingCommand.h"
#import "FIDOSettingWindow.h"
#import "ToolLogFile.h"

@interface FIDOSettingCommand ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) FIDOSettingWindow            *fidoSettingWindow;

@end

@implementation FIDOSettingCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 画面のインスタンスを生成
            [self setFidoSettingWindow:[[FIDOSettingWindow alloc] initWithWindowNibName:@"FIDOSettingWindow"]];
        }
        return self;
    }

    - (void)FIDOSettingWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self fidoSettingWindow] setParentWindowRef:parentWindow];
        [[self fidoSettingWindow] setCommandRef:self];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self fidoSettingWindow] window];
        FIDOSettingCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf fidoSettingWindowDidClose:self modalResponse:response];
        }];
    }

#pragma mark - Perform functions

    - (void)fidoSettingWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self fidoSettingWindow] close];
        // 実行コマンドにより処理分岐
        switch ([[self fidoSettingWindow] commandToPerform]) {
            default:
                // メイン画面に制御を戻す
                break;
        }
    }

@end
