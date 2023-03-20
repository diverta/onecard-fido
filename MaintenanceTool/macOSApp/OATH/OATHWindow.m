//
//  OATHWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/13.
//
#import "AccountSelectWindow.h"
#import "AppCommonMessage.h"
#import "OATHCommand.h"
#import "OATHWindow.h"
#import "QRCodeUtil.h"
#import "ScanQRCodeWindow.h"
#import "ToolCommonFunc.h"
#import "ToolPopupWindow.h"

@interface OATHWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 子画面の参照を保持
    @property (nonatomic) ScanQRCodeWindow             *scanQRCodeWindow;
    @property (nonatomic) AccountSelectWindow          *accountSelectWindow;
    // 画面項目を保持
    @property (assign) IBOutlet NSButton               *buttonTransportUSB;
    @property (assign) IBOutlet NSButton               *buttonTransportBLE;
    // コマンドクラス、パラメーターの参照を保持
    @property (assign) OATHCommand                     *oathCommand;
    @property (assign) OATHCommandParameter            *commandParameter;

@end

@implementation OATHWindow

    - (void)windowDidLoad {
        // コマンドクラスの初期化
        [self setOathCommand:[OATHCommand instance]];
        [self setCommandParameter:[[self oathCommand] parameter]];
        // 子画面の生成
        [self setScanQRCodeWindow:[[ScanQRCodeWindow alloc] initWithWindowNibName:@"ScanQRCodeWindow"]];
        [self setAccountSelectWindow:[[AccountSelectWindow alloc] initWithWindowNibName:@"AccountSelectWindow"]];
        // 画面項目の初期化
        [super windowDidLoad];
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // ラジオボタン「USB経由」を選択状態にする
        [[self buttonTransportUSB] setState:NSControlStateValueOn];
        [self buttonTransportSelected:[self buttonTransportUSB]];
    }

    - (IBAction)buttonTransportSelected:(id)sender {
        // トランスポート種別を設定
        if (sender == [self buttonTransportUSB]) {
            [[self commandParameter] setTransportType:TRANSPORT_HID];
        }
        if (sender == [self buttonTransportBLE]) {
            [[self commandParameter] setTransportType:TRANSPORT_BLE];
        }
    }

    - (IBAction)buttonScanQRCodeDidPress:(id)sender {
        // CCID I/F接続チェック
        if ([[self commandParameter] transportType] == TRANSPORT_HID) {
            if ([self checkUSBCCIDConnection] == false) {
                return;
            }
        }
        // TODO: BLEトランスポートをサポートするまでの暫定措置
        if ([[self commandParameter] transportType] == TRANSPORT_BLE) {
            [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil
                                           withObject:nil forSelector:nil parentWindow:[self window]];
            return;
        }
        // 画面収録の許可があるかどうかチェック
        if ([QRCodeUtil hasScreenshotPermission] == false) {
            NSString *bundleName = [ToolCommonFunc getAppBundleNameString];
            NSString *informative = [NSString stringWithFormat:MSG_INFORMATIVE_OATH_SCREENSHOT_PERMISSION, bundleName];
            [[ToolPopupWindow defaultWindow] critical:MSG_ERROR_OATH_SCREENSHOT_PERMISSION informativeText:informative
                                           withObject:nil forSelector:nil parentWindow:[self window]];
            return;
        }
        // 実行機能を設定し、画面を閉じる
        [[self commandParameter] setCommand:COMMAND_OATH_SCAN_QRCODE];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonShowPasswordDidPress:(id)sender {
        // CCID I/F接続チェック
        if ([[self commandParameter] transportType] == TRANSPORT_HID) {
            if ([self checkUSBCCIDConnection] == false) {
                return;
            }
        }
        // TODO: BLEトランスポートをサポートするまでの暫定措置
        if ([[self commandParameter] transportType] == TRANSPORT_BLE) {
            [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil
                                           withObject:nil forSelector:nil parentWindow:[self window]];
            return;
        }
        // アカウント選択画面を表示
        [[self commandParameter] setCommand:COMMAND_OATH_SHOW_PASSWORD];
        [self selectOATHAccountWithTitle:MSG_TITLE_OATH_ACCOUNT_SEL_FOR_TOTP caption:MSG_CAPTION_OATH_ACCOUNT_SEL_FOR_TOTP];
    }

    - (IBAction)buttonDeleteAccountDidPress:(id)sender {
        // CCID I/F接続チェック
        if ([[self commandParameter] transportType] == TRANSPORT_HID) {
            if ([self checkUSBCCIDConnection] == false) {
                return;
            }
        }
        // TODO: BLEトランスポートをサポートするまでの暫定措置
        if ([[self commandParameter] transportType] == TRANSPORT_BLE) {
            [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil
                                           withObject:nil forSelector:nil parentWindow:[self window]];
            return;
        }
        // アカウント選択画面を表示
        [[self commandParameter] setCommand:COMMAND_OATH_DELETE_ACCOUNT];
        [self selectOATHAccountWithTitle:MSG_TITLE_OATH_ACCOUNT_SEL_FOR_DELETE caption:MSG_CAPTION_OATH_ACCOUNT_SEL_FOR_DELETE];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

    - (bool)checkUSBCCIDConnection {
        // USB CCIDインターフェースに接続可能でない場合は処理中止
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:[self window] connected:[[self oathCommand] isUSBCCIDCanConnect]];
    }

#pragma mark - For OATH account selection

    - (void)selectOATHAccountWithTitle:(NSString *)title caption:(NSString *)caption {
        // OATH設定画面を開く
        [[self accountSelectWindow] windowWillOpenWithParentWindow:[self window] ForTarget:self forSelector:@selector(oathAccountDidSelect)];
    }

    - (void)oathAccountDidSelect {
        // 実行コマンドに応じ分岐
        switch ([[self commandParameter] command]) {
            case COMMAND_OATH_SHOW_PASSWORD:
                // 画面を閉じる
                [self terminateWindow:NSModalResponseOK];
                break;
            case COMMAND_OATH_DELETE_ACCOUNT:
                // TODO: 仮の実装です。
                [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil
                                               withObject:nil forSelector:nil parentWindow:[self window]];
                break;
            default:
                break;
        }
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
        // 実行機能をクリア
        [[self commandParameter] setCommand:COMMAND_NONE];
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
        // Cancelボタンクリック時は以降の処理を実行しない
        if (modalResponse == NSModalResponseCancel) {
            return;
        }
        // 実行コマンドにより処理分岐
        switch ([[self commandParameter] command]) {
            case COMMAND_OATH_SCAN_QRCODE:
                // 認証用QRコードスキャン画面を表示
                [[self scanQRCodeWindow] windowWillOpenWithParentWindow:[self parentWindow]];
                break;
            case COMMAND_OATH_SHOW_PASSWORD:
                // TODO: 仮の実装です。
                [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil
                                               withObject:nil forSelector:nil parentWindow:[self parentWindow]];
                break;
            default:
                // エラーメッセージをポップアップ表示
                [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil
                                               withObject:nil forSelector:nil parentWindow:[self parentWindow]];
                break;
        }
    }

@end
