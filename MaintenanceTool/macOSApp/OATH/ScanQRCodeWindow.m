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
        // TODO: 仮の実装です。
        NSString *informative = [NSString stringWithFormat:@"%@ \n%@ \n%@",
            [[[OATHCommand instance] parameter] oathAccountIssuer],
            [[[OATHCommand instance] parameter] oathAccountName],
            [[[OATHCommand instance] parameter] oathBase32Secret]
        ];
        [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:informative
                                       withObject:nil forSelector:nil parentWindow:[self window]];
    }

    - (IBAction)buttonUpdateDidPress:(id)sender {
        // TODO: 仮の実装です。
        [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil
                                       withObject:nil forSelector:nil parentWindow:[self window]];
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

@end
