//
//  ScanQRCodeWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/14.
//
#import "AppCommonMessage.h"
#import "OATHCommand.h"
#import "ScanQRCodeWindow.h"
#import "ToolPopupWindow.h"

@interface ScanQRCodeWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面項目を保持
    @property (assign) IBOutlet NSTextField            *labelTitle;
    @property (assign) IBOutlet NSTextField            *labelIssuerVal;
    @property (assign) IBOutlet NSTextField            *labelAccountVal;
    @property (assign) IBOutlet NSTextField            *labelPassword;
    @property (assign) IBOutlet NSButton               *buttonScan;
    @property (assign) IBOutlet NSButton               *buttonUpdate;

@end

@implementation ScanQRCodeWindow

    - (void)windowDidLoad {
        // 画面項目の初期化
        [super windowDidLoad];
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // ワンタイムパスワードの更新ボタンを使用不可とする
        [[self buttonScan] setEnabled:true];
        [[self buttonUpdate] setEnabled:false];
        // 画面表示項目を初期化
        [[self labelIssuerVal] setStringValue:@""];
        [[self labelAccountVal] setStringValue:@""];
        [[self labelPassword] setStringValue:@""];
    }

    - (IBAction)buttonScanDidPress:(id)sender {
        // 画面項目の初期化
        [self initFieldValue];
        // QRコードのスキャンを画面スレッドで実行
        if ([[OATHCommand instance] scanQRCode] == false) {
            NSString *informative = [[[OATHCommand instance] parameter] resultInformativeMessage];
            [[ToolPopupWindow defaultWindow] critical:[[self labelTitle] stringValue] informativeText:informative
                                           withObject:nil forSelector:nil parentWindow:[self window]];
            return;
        }
        // ワンタイムパスワードを生成
        [self doOATHProcessWithCommandTitle:MSG_LABEL_COMMAND_OATH_GENERATE_TOTP];
    }

    - (IBAction)buttonUpdateDidPress:(id)sender {
        // ワンタイムパスワードを生成
        [self doOATHProcessWithCommandTitle:MSG_LABEL_COMMAND_OATH_UPDATE_TOTP];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

#pragma mark - For ScanQRCodeWindow open/close

    - (bool)windowWillOpenWithParentWindow:(NSWindow *)parent {
        // 親画面の参照を保持
        [self setParentWindow:parent];
        // すでにダイアログが開いている場合は終了
        if ([[[self parentWindow] sheets] count] > 0) {
            return false;
        }
        // すでにダイアログがロード済みの場合は、画面項目を再度初期化
        if ([self isWindowLoaded]) {
            [self initFieldValue];
        }
        // ダイアログをモーダルで表示
        NSWindow *dialog = [self window];
        ScanQRCodeWindow * __weak weakSelf = self;
        [[self parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf windowDidCloseWithModalResponse:response];
        }];
        return true;
    }

    - (void)windowDidCloseWithModalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [self close];
    }

#pragma mark - Private functions

    - (void)doOATHProcessWithCommandTitle:(NSString *)commandTitle {
        // パラメーターを設定し、コマンドを実行
        [[[OATHCommand instance] parameter] setCommandTitle:commandTitle];
        [[OATHCommand instance] commandWillPerformForTarget:self forSelector:@selector(oathProcessDidTerminated)];
    }

    - (void)oathProcessDidTerminated {
        // 処理失敗時は、エラーメッセージをポップアップ表示
        if ([[[OATHCommand instance] parameter] commandSuccess] == false) {
            NSString *message = [[[OATHCommand instance] parameter] resultMessage];
            NSString *informative = [[[OATHCommand instance] parameter] resultInformativeMessage];
            [[ToolPopupWindow defaultWindow] critical:message informativeText:informative
                                           withObject:nil forSelector:nil parentWindow:[self window]];
        }
        // アカウント情報の各項目を画面表示
        [[self labelAccountVal] setStringValue:[[[OATHCommand instance] parameter] oathAccountName]];
        [[self labelIssuerVal] setStringValue:[[[OATHCommand instance] parameter] oathAccountIssuer]];
        NSString *totp = [NSString stringWithFormat:@"%06d", [[[OATHCommand instance] parameter] oathTotpValue]];
        [[self labelPassword] setStringValue:totp];
        // 実行ボタンの代わりに、更新ボタンを使用可能とする
        [[self buttonScan] setEnabled:false];
        [[self buttonUpdate] setEnabled:true];
    }

@end
