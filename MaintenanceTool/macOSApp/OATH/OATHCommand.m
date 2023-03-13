//
//  OATHCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/13.
//
#import "AppCommonMessage.h"
#import "OATHCommand.h"
#import "OATHWindow.h"
#import "ToolPopupWindow.h"

@implementation OATHCommandParameter

@end

@interface OATHCommand ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                 *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) OATHWindow               *oathWindow;
    // 処理のパラメーターを保持
    @property (nonatomic) OATHCommandParameter     *commandParameter;

@end

@implementation OATHCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // 画面のインスタンスを生成
            [self setOathWindow:[[OATHWindow alloc] initWithWindowNibName:@"OATHWindow"]];
            // ヘルパークラスのインスタンスを生成
            [self setCommandParameter:[[OATHCommandParameter alloc] init]];
        }
        return self;
    }

    - (void)oathWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self oathWindow] setParentWindowRef:parentWindow];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self oathWindow] window];
        OATHCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf oathWindowDidClose:self modalResponse:response];
        }];
    }

#pragma mark - Perform functions

    - (void)oathWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self oathWindow] close];
        if (modalResponse == NSModalResponseCancel) {
            return;
        }
        // 実行コマンドにより処理分岐
        switch ([[self commandParameter] command]) {
            default:
                [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil
                                               withObject:self forSelector:nil parentWindow:[self parentWindow]];
                break;
        }
    }

@end
