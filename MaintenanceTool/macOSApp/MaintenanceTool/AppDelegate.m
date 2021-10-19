//
//  AppDelegate.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/14.
//
#import "AppDelegate.h"
#import "ToolAppCommand.h"
#import "ToolContext.h"
#import "ToolFilePanel.h"
#import "ToolPopupWindow.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

@interface AppDelegate () <ToolAppCommandDelegate, ToolFilePanelDelegate>

    @property (assign) IBOutlet NSWindow    *window;
    @property (assign) IBOutlet NSButton    *button1;
    @property (assign) IBOutlet NSButton    *button2;
    @property (assign) IBOutlet NSButton    *button3;
    @property (assign) IBOutlet NSButton    *button4;
    @property (assign) IBOutlet NSButton    *buttonQuit;
    @property (assign) IBOutlet NSTextView  *textView;

    @property (assign) IBOutlet NSTextField *fieldPath1;
    @property (assign) IBOutlet NSTextField *fieldPath2;
    @property (assign) IBOutlet NSButton    *buttonPath1;
    @property (assign) IBOutlet NSButton    *buttonPath2;

    @property (assign) IBOutlet NSMenuItem  *menuItemTestUSB;
    @property (assign) IBOutlet NSMenuItem  *menuItemTestBLE;
    @property (assign) IBOutlet NSMenuItem  *menuItemOption;
    @property (assign) IBOutlet NSMenuItem  *menuItemEraseBond;
    @property (assign) IBOutlet NSMenuItem  *menuItemPreferences;
    @property (assign) IBOutlet NSMenuItem  *menuItemViewLog;
    @property (assign) IBOutlet NSMenuItem  *menuItemUSBDFU;
    @property (assign) IBOutlet NSMenuItem  *menuItemBLEDFU;

    @property (nonatomic) ToolAppCommand    *toolAppCommand;
    @property (nonatomic) ToolFilePanel     *toolFilePanel;
@end

@implementation AppDelegate

    - (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
        // 共有情報にアプリケーションの参照を設定
        [[ToolContext instance] setAppDelegateRef:self];

        // アプリケーション開始ログを出力
        [[ToolLogFile defaultLogger] infoWithFormat:MSG_APP_LAUNCHED, [ToolCommon getAppVersionString]];

        // コマンドクラスの初期化
        [self setToolAppCommand:[[ToolAppCommand alloc] initWithDelegate:self]];
        [self setToolFilePanel:[[ToolFilePanel alloc] initWithDelegate:self]];

        // テキストエリアの初期化
        [[self textView] setFont:[NSFont fontWithName:@"Courier" size:12]];
    }

    - (void)applicationWillTerminate:(NSNotification *)notification {
        // アプリケーションの終了ログを出力
        [[ToolLogFile defaultLogger] info:MSG_APP_TERMINATED];
    }

    - (void)appendLogMessage:(NSString *)message {
        // テキストフィールドにメッセージを追加し、末尾に移動
        if (message) {
            [[self textView] setString:[[[self textView] string] stringByAppendingFormat:@"%@\n", message]];
            [[self textView] performSelector:@selector(scrollToEndOfDocument:) withObject:nil afterDelay:0];
        }
    }

#pragma mark - Functions for button handling

    - (void)enableButtons:(bool)enabled {
        // ボタンや入力欄の使用可能／不可制御
        [[self button1] setEnabled:enabled];
        [[self button2] setEnabled:enabled];
        [[self button3] setEnabled:enabled];
        [[self button4] setEnabled:enabled];
        [[self fieldPath1] setEnabled:enabled];
        [[self fieldPath2] setEnabled:enabled];
        [[self buttonPath1] setEnabled:enabled];
        [[self buttonPath2] setEnabled:enabled];
        [[self buttonQuit] setEnabled:enabled];
        [[self menuItemTestUSB] setEnabled:enabled];
        [[self menuItemTestBLE] setEnabled:enabled];
        [[self menuItemOption] setEnabled:enabled];
        [[self menuItemPreferences] setHidden:!(enabled)];
        [[self menuItemViewLog] setEnabled:enabled];
        [[self menuItemEraseBond] setEnabled:enabled];
        [[self menuItemUSBDFU] setEnabled:enabled];
        [[self menuItemBLEDFU] setEnabled:enabled];
    }

    - (IBAction)button1DidPress:(id)sender {
        // ペアリング実行
        [[self toolAppCommand] doCommandPairing];
    }

    - (IBAction)button2DidPress:(id)sender {
        // 鍵・証明書削除
        [[self toolAppCommand] doCommandEraseSkeyCert];
    }

    - (bool)checkPathEntry:(NSTextField *)field messageIfError:(NSString *)message {
        // 入力項目が正しく指定されていない場合は終了
        if ([ToolCommon checkMustEntry:field informativeText:message] == false) {
            return false;
        }
        // 入力されたファイルパスが存在しない場合は終了
        if ([ToolCommon checkFileExist:field informativeText:message] == false) {
            return false;
        }
        return true;
    }

    - (IBAction)button3DidPress:(id)sender {
        if ([self checkPathEntry:[self fieldPath1] messageIfError:MSG_PROMPT_SELECT_PKEY_PATH] == false) {
            return;
        }
        if ([self checkPathEntry:[self fieldPath2] messageIfError:MSG_PROMPT_SELECT_CRT_PATH] == false) {
            return;
        }
        NSArray<NSString *> *paths = @[[[self fieldPath1] stringValue], [[self fieldPath2] stringValue]];
        // 鍵・証明書インストール
        [[self toolAppCommand] doCommandInstallSkeyCert:paths];
    }

    - (IBAction)button4DidPress:(id)sender {
        // PINコード設定画面を開く
        [[self toolAppCommand] setPinParamWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)buttonQuitDidPress:(id)sender {
        // このアプリケーションを終了する
        [NSApp terminate:sender];
    }

    - (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
        // ウィンドウをすべて閉じたらアプリケーションを終了
        return YES;
    }

    - (IBAction)buttonPath1DidPress:(id)sender {
        [self enableButtons:false];
        [[self toolFilePanel] panelWillSelectPath:sender parentWindow:[self window]
                                       withPrompt:MSG_BUTTON_SELECT withMessage:MSG_PROMPT_SELECT_PKEY_PATH withFileTypes:@[@"pem"]];
    }

    - (IBAction)buttonPath2DidPress:(id)sender {
        [self enableButtons:false];
        [[self toolFilePanel] panelWillSelectPath:sender parentWindow:[self window]
                                       withPrompt:MSG_BUTTON_SELECT withMessage:MSG_PROMPT_SELECT_CRT_PATH withFileTypes:@[@"crt"]];
    }

    - (IBAction)menuItemTestHID1DidSelect:(id)sender {
        // HID CTAP2ヘルスチェック実行
        [[self toolAppCommand] doCommandHidCtap2HealthCheck];
    }

    - (IBAction)menuItemTestHID2DidSelect:(id)sender {
        // HID U2Fヘルスチェック実行
        [[self toolAppCommand] doCommandHidU2fHealthCheck];
    }

    - (IBAction)menuItemTestHID3DidSelect:(id)sender {
        // PINGテスト実行
        [[self toolAppCommand] doCommandTestCtapHidPing];
    }

    - (IBAction)menuItemTestHID4DidSelect:(id)sender {
        // Flash ROM情報取得
        [[self toolAppCommand] doCommandHidGetFlashStat];
    }

    - (IBAction)menuItemTestHID5DidSelect:(id)sender {
        // バージョン情報取得
        [[self toolAppCommand] doCommandHidGetVersionInfo];
    }

    - (IBAction)menuItemTestBLE1DidSelect:(id)sender {
        // BLE CTAP2ヘルスチェック実行（PINコード入力画面を開く）
        [[self toolAppCommand] pinCodeParamWindowWillOpenForBLE:self parentWindow:[self window]];
    }

    - (IBAction)menuItemTestBLE2DidSelect:(id)sender {
        // BLE U2Fヘルスチェック実行
        [[self toolAppCommand] doCommandTestRegister];
    }

    - (IBAction)menuItemTestBLE3DidSelect:(id)sender {
        // BLE PINGテスト実行
        [[self toolAppCommand] doCommandTestBlePing];
    }

    - (IBAction)menuItemOptionPivSettingsDidSelect:(id)sender {
        // PIV機能設定画面を表示
        [[self toolAppCommand] PreferenceWindowWillOpenWithParent:[self window]];
    }

    - (IBAction)menuItemPreferencesDidSelect:(id)sender {
        // ツール設定画面を開く
        [[self toolAppCommand] toolPreferenceWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)menuItemViewLogDidSelect:(id)sender {
        // ログファイル格納ディレクトリーをFinderで表示
        NSURL *url =
        [NSURL fileURLWithPath:[[ToolLogFile defaultLogger] logFilePathString] isDirectory:false];
        NSArray *fileURLs = [NSArray arrayWithObjects:url, nil];
        [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:fileURLs];
    }

    - (IBAction)menuItemDFUTestDidSelect:(id)sender {
        [[self toolAppCommand] dfuProcessWillStart:self parentWindow:[self window]];
    }

    - (IBAction)menuItemDFUNewDidSelect:(id)sender {
        [[self toolAppCommand] dfuNewProcessWillStart:self parentWindow:[self window]];
    }

    - (IBAction)menuItemEraseBondDidSelect:(id)sender {
        // ペアリング情報削除
        [[self toolAppCommand] doCommandEraseBond];
    }

    - (IBAction)menuItemBLModeDidSelect:(id)sender {
        // ブートローダーモード遷移
        [[self toolAppCommand] doCommandBLMode];
    }

    - (IBAction)menuItemBLEDFUDidSelect:(id)sender {
        // ファームウェア更新をBLE経由で実行
        [[self toolAppCommand] doCommandBLEDFU];
    }

#pragma mark - Call back from ToolFilePanel

    - (void)panelDidSelectPath:(id)sender filePath:(NSString*)filePath
                 modalResponse:(NSInteger)modalResponse {
        // OKボタン押下時は、ファイル選択パネルで選択されたファイルパスを表示する
        if (modalResponse == NSFileHandlingPanelOKButton) {
            if ([self buttonPath1] == sender) {
                [[self fieldPath1] setStringValue:filePath];
                [[self fieldPath1] setToolTip:filePath];
            }
            if ([self buttonPath2] == sender) {
                [[self fieldPath2] setStringValue:filePath];
                [[self fieldPath2] setToolTip:filePath];
            }
        }
        // メニューを活性化
        [self enableButtons:true];
    }

#pragma mark - Call back from ToolAppCommand

    - (void)disableUserInterface {
        // メニュー、ボタンを非活性化
        [self enableButtons:false];
    }

    - (void)notifyAppCommandMessage:(NSString *)message {
        // 画面上のテキストエリアにメッセージを表示する
        if (message) {
            [self appendLogMessage:message];
        }
    }

    - (void)pinCodeParamWindowWillOpenForHID {
        // HID CTAP2ヘルスチェック処理を実行（PINコード入力画面を開く）
        [[self toolAppCommand] pinCodeParamWindowWillOpenForHID:self parentWindow:[self window]];
    }

#pragma mark - Common method called by callback

    - (void)commandStartedProcess:(NSString *)processNameOfCommand {
        if (processNameOfCommand) {
            // コマンド開始メッセージを画面表示し、ログファイルにも出力
            NSString *startMsg = [NSString stringWithFormat:MSG_FORMAT_START_MESSAGE,
                                  processNameOfCommand];
            [self notifyAppCommandMessage:startMsg];
            [[ToolLogFile defaultLogger] info:startMsg];
        }
    }

    - (void)commandDidProcess:(bool)result message:(NSString *)message processNameOfCommand:(NSString *)processNameOfCommand {
        // 処理失敗時は、引数に格納されたエラーメッセージを画面出力
        if (result == false) {
            [self notifyAppCommandMessage:message];
        }
        // コマンド名称を取得
        if (processNameOfCommand) {
            // テキストエリアとポップアップの両方に表示させる処理終了メッセージを作成
            NSString *str = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE,
                             processNameOfCommand, result? MSG_SUCCESS:MSG_FAILURE];
            // メッセージを画面のテキストエリアに表示
            [self notifyAppCommandMessage:str];
            // メッセージをログファイルに出力してから、ポップアップを表示
            if (result) {
                [[ToolLogFile defaultLogger] info:str];
                [ToolPopupWindow informational:str informativeText:nil];
            } else {
                [[ToolLogFile defaultLogger] error:str];
                [ToolPopupWindow critical:str informativeText:nil];
            }
        }
        // ボタンを活性化
        [self enableButtons:true];
    }

@end
