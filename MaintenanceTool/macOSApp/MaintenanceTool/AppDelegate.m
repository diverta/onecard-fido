//
//  AppDelegate.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/14.
//
#import "AppDelegate.h"
#import "ToolAppCommand.h"
#import "ToolContext.h"
#import "ToolPopupWindow.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

@interface AppDelegate () <ToolAppCommandDelegate>

    @property (assign) IBOutlet NSWindow    *window;
    @property (assign) IBOutlet NSButton    *buttonPairing;
    @property (assign) IBOutlet NSButton    *buttonUnpairing;
    @property (assign) IBOutlet NSButton    *buttonFIDOAttestation;
    @property (assign) IBOutlet NSButton    *buttonSetPinParam;
    @property (assign) IBOutlet NSButton    *buttonSetPivParam;
    @property (assign) IBOutlet NSButton    *buttonDFU;
    @property (assign) IBOutlet NSButton    *buttonSetPgpParam;
    @property (assign) IBOutlet NSButton    *buttonQuit;
    @property (assign) IBOutlet NSTextView  *textView;

    @property (assign) IBOutlet NSMenuItem  *menuItemTestUSB;
    @property (assign) IBOutlet NSMenuItem  *menuItemTestBLE;
    @property (assign) IBOutlet NSMenuItem  *menuItemPreferences;
    @property (assign) IBOutlet NSMenuItem  *menuItemViewLog;

    @property (nonatomic) ToolAppCommand    *toolAppCommand;
@end

@implementation AppDelegate

    - (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
        // 共有情報にアプリケーションの参照を設定
        [[ToolContext instance] setAppDelegateRef:self];

        // アプリケーション開始ログを出力
        [[ToolLogFile defaultLogger] infoWithFormat:MSG_APP_LAUNCHED, [ToolCommon getAppVersionString]];

        // コマンドクラスの初期化
        [self setToolAppCommand:[[ToolAppCommand alloc] initWithDelegate:self]];

        // テキストエリアの初期化
        [[self textView] setFont:[NSFont fontWithName:@"Courier" size:12]];
        
        // 共通ポップアップ画面の初期化
        [[ToolPopupWindow defaultWindow] setApplicationWindow:[self window]];
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
        [[self buttonPairing] setEnabled:enabled];
        [[self buttonUnpairing] setEnabled:enabled];
        [[self buttonFIDOAttestation] setEnabled:enabled];
        [[self buttonSetPinParam] setEnabled:enabled];
        [[self buttonSetPivParam] setEnabled:enabled];
        [[self buttonDFU] setEnabled:enabled];
        [[self buttonSetPgpParam] setEnabled:enabled];
        [[self buttonQuit] setEnabled:enabled];
        [[self menuItemTestUSB] setEnabled:enabled];
        [[self menuItemTestBLE] setEnabled:enabled];
        [[self menuItemPreferences] setHidden:!(enabled)];
        [[self menuItemViewLog] setEnabled:enabled];
    }

    - (IBAction)buttonPairingDidPress:(id)sender {
        // ペアリング実行
        [[self toolAppCommand] doCommandPairing];
    }

    - (IBAction)buttonUnpairingDidPress:(id)sender {
        // ペアリング情報削除
        [[self toolAppCommand] doCommandEraseBond];
    }

    - (IBAction)buttonFIDOAttestationDidPress:(id)sender {
        // FIDO鍵・証明書設定画面を開く
        [[self toolAppCommand] fidoAttestationWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)buttonSetPinParamDidPress:(id)sender {
        // PINコード設定画面を開く
        [[self toolAppCommand] setPinParamWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)buttonSetPivParamDidPress:(id)sender {
        // PIV機能設定画面を表示
        [[self toolAppCommand] PreferenceWindowWillOpenWithParent:[self window]];
    }

    - (IBAction)buttonDFUDidPress:(id)sender {
        // ファームウェア更新画面を表示
        [[self toolAppCommand] toolDFUWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)buttonSetPgpParamDidPress:(id)sender {
        // OpenPGP機能設定画面を表示
        [[self toolAppCommand] pgpParamWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)buttonQuitDidPress:(id)sender {
        // このアプリケーションを終了する
        [NSApp terminate:sender];
    }

    - (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
        // ウィンドウをすべて閉じたらアプリケーションを終了
        return YES;
    }

    - (IBAction)menuItemTestHID1DidSelect:(id)sender {
        // HID CTAP2ヘルスチェック実行
        [[self toolAppCommand] doCommandHidCtap2HealthCheck:[self window]];
    }

    - (IBAction)menuItemTestHID2DidSelect:(id)sender {
        // HID U2Fヘルスチェック実行
        [[self toolAppCommand] doCommandHidU2fHealthCheck:[self window]];
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
        [[self toolAppCommand] doCommandBleCtap2HealthCheck:[self window]];
    }

    - (IBAction)menuItemTestBLE2DidSelect:(id)sender {
        // BLE U2Fヘルスチェック実行
        [[self toolAppCommand] doCommandBleU2fHealthCheck:[self window]];
    }

    - (IBAction)menuItemTestBLE3DidSelect:(id)sender {
        // BLE PINGテスト実行
        [[self toolAppCommand] doCommandTestBlePing];
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

#pragma mark - Call back from ToolAppCommand

    - (void)disableUserInterface {
        // メニュー、ボタンを非活性化
        [self enableButtons:false];
    }

    - (void)enableUserInterface {
        // メニュー、ボタンを活性化
        [self enableButtons:true];
    }

    - (void)notifyAppCommandMessage:(NSString *)message {
        // 画面上のテキストエリアにメッセージを表示する
        if (message) {
            [self appendLogMessage:message];
        }
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
        if (processNameOfCommand == nil) {
            // ボタンを活性化
            [self enableButtons:true];
            return;
        }
        // テキストエリアとポップアップの両方に表示させる処理終了メッセージを作成
        NSString *str = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE,
                         processNameOfCommand, result? MSG_SUCCESS:MSG_FAILURE];
        // メッセージを画面のテキストエリアに表示
        [self notifyAppCommandMessage:str];
        // メッセージをログファイルに出力してから、ポップアップを表示
        [self displayCommandResult:result withMessage:str];
    }

    - (void)displayCommandResult:(bool)result withMessage:(NSString *)str{
        // メッセージをログファイルに出力してから、ポップアップを表示-->ボタンを活性化
        if (result) {
            [[ToolLogFile defaultLogger] info:str];
            [[ToolPopupWindow defaultWindow] informational:str informativeText:nil withObject:self forSelector:@selector(displayCommandResultDone)];
        } else {
            [[ToolLogFile defaultLogger] error:str];
            [[ToolPopupWindow defaultWindow] critical:str informativeText:nil withObject:self forSelector:@selector(displayCommandResultDone)];
        }
    }

    - (void)displayCommandResultDone {
        // ボタンを活性化
        [self enableButtons:true];
    }

@end
