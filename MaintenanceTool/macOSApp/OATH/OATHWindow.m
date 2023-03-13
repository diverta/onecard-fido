//
//  OATHWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/13.
//
#import "OATHCommand.h"
#import "OATHWindow.h"
#import "ToolCommonFunc.h"

@interface OATHWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面項目を保持
    @property (assign) IBOutlet NSButton               *buttonTransportUSB;
    @property (assign) IBOutlet NSButton               *buttonTransportBLE;

@end

@implementation OATHWindow

    - (void)windowDidLoad {
        // 画面項目の初期化
        [super windowDidLoad];
        [self initFieldValue];
        // コマンドクラスの初期化
        [OATHCommand instance];
    }

    - (void)initFieldValue {
        // ラジオボタン「USB経由」を選択状態にする
        [[self buttonTransportUSB] setState:NSControlStateValueOn];
        [self buttonTransportSelected:[self buttonTransportUSB]];
    }

    - (IBAction)buttonTransportSelected:(id)sender {
        // トランスポート種別を設定
        if (sender == [self buttonTransportUSB]) {
            [[[OATHCommand instance] parameter] setTransportType:TRANSPORT_HID];
        }
        if (sender == [self buttonTransportBLE]) {
            [[[OATHCommand instance] parameter] setTransportType:TRANSPORT_BLE];
        }
    }

    - (IBAction)buttonScanQRCodeDidPress:(id)sender {
        // CCID I/F接続チェック
        if ([[[OATHCommand instance] parameter] transportType] == TRANSPORT_HID) {
            if ([self checkUSBHIDConnection] == false) {
                return;
            }
        }
    }

    - (IBAction)buttonShowPasswordDidPress:(id)sender {
        // CCID I/F接続チェック
        if ([[[OATHCommand instance] parameter] transportType] == TRANSPORT_HID) {
            if ([self checkUSBHIDConnection] == false) {
                return;
            }
        }
    }

    - (IBAction)buttonDeleteAccountDidPress:(id)sender {
        // CCID I/F接続チェック
        if ([[[OATHCommand instance] parameter] transportType] == TRANSPORT_HID) {
            if ([self checkUSBHIDConnection] == false) {
                return;
            }
        }
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

    - (bool)checkUSBHIDConnection {
        // USB CCIDインターフェースに接続可能でない場合は処理中止
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:[self window] connected:[[OATHCommand instance] isUSBCCIDCanConnect]];
    }

#pragma mark - For OATHWindow open/close

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
        OATHWindow * __weak weakSelf = self;
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
