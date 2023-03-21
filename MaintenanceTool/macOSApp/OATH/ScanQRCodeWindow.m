//
//  ScanQRCodeWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/14.
//
#import "AppCommonMessage.h"
#import "OATHCommand.h"
#import "OATHWindowUtil.h"
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
    // コマンドクラス、パラメーターの参照を保持
    @property (assign) OATHCommand                     *oathCommand;
    @property (assign) OATHCommandParameter            *commandParameter;

@end

@implementation ScanQRCodeWindow

    - (void)windowDidLoad {
        // コマンドクラス、パラメーターの参照を保持
        [self setOathCommand:[OATHCommand instance]];
        [self setCommandParameter:[[self oathCommand] parameter]];
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
        if ([[self oathCommand] scanQRCode] == false) {
            NSString *informative = [[self commandParameter] resultInformativeMessage];
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
        // アプリケーションをフローティング表示状態に変更
        [[self parentWindow] setLevel:NSFloatingWindowLevel];
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
        // アプリケーションを通常のウィンドウ状態に戻す
        [[self parentWindow] setLevel:NSNormalWindowLevel];
    }

#pragma mark - Private functions

    - (void)doOATHProcessWithCommandTitle:(NSString *)commandTitle {
        // パラメーターを設定し、コマンドを実行
        [[self commandParameter] setCommandTitle:commandTitle];
        [[[OATHWindowUtil alloc] init] commandWillPerformForTarget:self forSelector:@selector(oathProcessDidTerminated) withParentWindow:[self window]];
    }

    - (void)oathProcessDidTerminated {
        // 処理失敗時は以降の処理を行わない
        if ([[self commandParameter] commandSuccess] == false) {
            return;
        }
        // アカウント情報の各項目を画面表示
        [[self labelAccountVal] setStringValue:[[self commandParameter] oathAccountName]];
        [[self labelIssuerVal] setStringValue:[[self commandParameter] oathAccountIssuer]];
        NSString *totp = [NSString stringWithFormat:@"%06d", [[self commandParameter] oathTotpValue]];
        [[self labelPassword] setStringValue:totp];
        // 実行ボタンの代わりに、更新ボタンを使用可能とする
        [[self buttonScan] setEnabled:false];
        [[self buttonUpdate] setEnabled:true];
    }

@end
