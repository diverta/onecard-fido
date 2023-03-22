//
//  TOTPDisplayWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/22.
//
#import "AppCommonMessage.h"
#import "OATHCommand.h"
#import "OATHWindowUtil.h"
#import "TOTPDisplayWindow.h"

@interface TOTPDisplayWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // パラメーターの参照を保持
    @property (nonatomic) OATHCommandParameter         *commandParameter;
    // 画面項目（アカウント／ワンタイムパスワード）に表示するデータを保持
    @property (nonatomic) NSString                     *accountIssuerString;
    @property (nonatomic) NSString                     *accountNameString;
    @property (nonatomic) NSString                     *totpString;

@end

@implementation TOTPDisplayWindow

    - (void)windowDidLoad {
        // 画面項目の初期化
        [super windowDidLoad];
    }

    - (IBAction)buttonUpdateDidPress:(id)sender {
        // ワンタイムパスワードを認証器で生成
        [[self commandParameter] setCommandTitle:MSG_LABEL_COMMAND_OATH_UPDATE_TOTP];
        [[[OATHWindowUtil alloc] init] commandWillPerformForTarget:self forSelector:@selector(displayTOTP) withParentWindow:[self window]];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

#pragma mark - For TOTP display

    - (void)displayTOTP {
        // 画面項目（アカウント／ワンタイムパスワード）に表示
        if ([[self commandParameter] commandSuccess]) {
            [self displayAccountInfo];
        }
    }

    - (void)displayAccountInfo {
        // 画面項目（アカウント／ワンタイムパスワード）に表示
        [self setAccountIssuerString:[[self commandParameter] oathAccountIssuer]];
        [self setAccountNameString:[[self commandParameter] oathAccountName]];
        [self setTotpString:[NSString stringWithFormat:@"%06d", [[self commandParameter] oathTotpValue]]];
    }

#pragma mark - For TOTPDisplayWindow open/close

    - (bool)windowWillOpenWithParentWindow:(NSWindow *)parent {
        // パラメーターの参照を保持
        [self setCommandParameter:[[OATHCommand instance] parameter]];
        // 親画面の参照を保持
        [self setParentWindow:parent];
        // すでにダイアログが開いている場合は終了
        if ([[[self parentWindow] sheets] count] > 0) {
            return false;
        }
        // 画面項目（アカウント／ワンタイムパスワード）に表示
        [self displayAccountInfo];
        // アプリケーションをフローティング表示状態に変更
        [[self parentWindow] setLevel:NSFloatingWindowLevel];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [self window];
        TOTPDisplayWindow * __weak weakSelf = self;
        [[self parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf windowDidCloseWithModalResponse:response];
        }];
        return true;
    }

    - (void)windowDidCloseWithModalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [self close];
        // アプリケーションを通常のウィンドウ状態に戻す
        [[self parentWindow] setLevel:NSNormalWindowLevel];
    }

@end
