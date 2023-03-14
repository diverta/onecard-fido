//
//  ScanQRCodeWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/14.
//
#import "ScanQRCodeWindow.h"

@interface ScanQRCodeWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面項目を保持
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
    }

    - (IBAction)buttonScanDidPress:(id)sender {
    }

    - (IBAction)buttonUpdateDidPress:(id)sender {
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
