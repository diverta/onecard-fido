#import "AppDelegate.h"
#import "ToolContext.h"
#import "ToolHIDCommand.h"
#import "ToolBLECommand.h"
#import "ToolFilePanel.h"
#import "ToolPopupWindow.h"
#import "ToolCommonMessage.h"
#import "ToolPreferenceCommand.h"
#import "ToolLogFile.h"
#import "ToolDFUCommand.h"

@interface AppDelegate ()
    <ToolHIDCommandDelegate, ToolBLECommandDelegate, ToolFilePanelDelegate>

    @property (assign) IBOutlet NSWindow   *window;
    @property (assign) IBOutlet NSButton   *button1;
    @property (assign) IBOutlet NSButton   *button2;
    @property (assign) IBOutlet NSButton   *button3;
    @property (assign) IBOutlet NSButton   *button4;
    @property (assign) IBOutlet NSButton   *buttonQuit;
    @property (assign) IBOutlet NSTextView *textView;

    @property (assign) IBOutlet NSTextField *fieldPath1;
    @property (assign) IBOutlet NSTextField *fieldPath2;
    @property (assign) IBOutlet NSButton    *buttonPath1;
    @property (assign) IBOutlet NSButton    *buttonPath2;

    @property (assign) IBOutlet NSMenuItem  *menuItemTestUSB;
    @property (assign) IBOutlet NSMenuItem  *menuItemTestBLE;
    @property (assign) IBOutlet NSMenuItem  *menuItemEraseBond;
    @property (assign) IBOutlet NSMenuItem  *menuItemBLMode;
    @property (assign) IBOutlet NSMenuItem  *menuItemPreferences;
    @property (assign) IBOutlet NSMenuItem  *menuItemViewLog;
    @property (assign) IBOutlet NSMenuItem  *menuItemDFU;
    @property (assign) IBOutlet NSMenuItem  *menuItemDFUNew;

    @property (nonatomic) ToolBLECommand    *toolBLECommand;
    @property (nonatomic) ToolHIDCommand    *toolHIDCommand;
    @property (nonatomic) ToolFilePanel     *toolFilePanel;
    @property (nonatomic) ToolPreferenceCommand *toolPreferenceCommand;
    @property (nonatomic) ToolDFUCommand    *toolDFUCommand;

    // 処理機能名称を保持
    @property (nonatomic) NSString *processNameOfCommand;

    // 実行するヘルスチェックの種別を保持
    @property (nonatomic) Command   healthCheckCommand;
@end

@implementation AppDelegate

    - (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
        // 共有情報にアプリケーションの参照を設定
        [[ToolContext instance] setAppDelegateRef:self];

        // アプリケーション開始ログを出力
        [[ToolLogFile defaultLogger] infoWithFormat:MSG_APP_LAUNCHED, [ToolCommon getAppVersionString]];

        // コマンドクラスの初期化
        [self setToolHIDCommand:[[ToolHIDCommand alloc] initWithDelegate:self]];
        [self setToolBLECommand:[[ToolBLECommand alloc] initWithDelegate:self]];
        [self setToolFilePanel:[[ToolFilePanel alloc] initWithDelegate:self]];

        // テキストエリアの初期化
        [[self textView] setFont:[NSFont fontWithName:@"Courier" size:12]];

        // 設定画面の初期設定
        [self setToolPreferenceCommand:[[ToolPreferenceCommand alloc] initWithDelegate:self]];
        
        // DFU機能の初期設定
        [self setToolDFUCommand:[[ToolDFUCommand alloc] initWithDelegate:self]];
    }

    - (void)applicationWillTerminate:(NSNotification *)notification {
        // アプリケーションの終了ログを出力
        [[ToolLogFile defaultLogger] info:MSG_APP_TERMINATED];
    }

    - (void)appendLogMessage:(NSString *)message {
        // テキストフィールドにメッセージを追加し、末尾に移動
        if (message) {
            self.textView.string = [self.textView.string stringByAppendingFormat:@"%@\n", message];
            [self.textView performSelector:@selector(scrollToEndOfDocument:) withObject:nil afterDelay:0];
        }
    }

#pragma mark - Functions for button handling

    - (void)enableButtons:(bool)enabled {
        // ボタンや入力欄の使用可能／不可制御
        [self.button1 setEnabled:enabled];
        [self.button2 setEnabled:enabled];
        [self.button3 setEnabled:enabled];
        [self.button4 setEnabled:enabled];
        [self.fieldPath1 setEnabled:enabled];
        [self.fieldPath2 setEnabled:enabled];
        [self.buttonPath1 setEnabled:enabled];
        [self.buttonPath2 setEnabled:enabled];
        [self.buttonQuit setEnabled:enabled];
        [self.menuItemTestUSB setEnabled:enabled];
        [self.menuItemTestBLE setEnabled:enabled];
        [self.menuItemPreferences setHidden:!(enabled)];
        [self.menuItemViewLog setEnabled:enabled];
        [self.menuItemDFU setEnabled:enabled];
        [self.menuItemDFUNew setEnabled:enabled];
        [self.menuItemEraseBond setEnabled:enabled];
        [self.menuItemBLMode setEnabled:enabled];
    }

    - (IBAction)button1DidPress:(id)sender {
        // ペアリング実行
        [self enableButtons:false];
        [[self toolBLECommand] bleCommandWillProcess:COMMAND_PAIRING];
    }

    - (IBAction)button2DidPress:(id)sender {
        if (![self checkUSBHIDConnection]) {
            return;
        }
        // 鍵・証明書削除
        if ([ToolPopupWindow promptYesNo:MSG_ERASE_SKEY_CERT
                         informativeText:MSG_PROMPT_ERASE_SKEY_CERT] == false) {
            return;
        }
        [self enableButtons:false];
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_ERASE_SKEY_CERT];
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
        if ([self checkPathEntry:self.fieldPath1 messageIfError:MSG_PROMPT_SELECT_PKEY_PATH] == false) {
            return;
        }
        if ([self checkPathEntry:self.fieldPath2 messageIfError:MSG_PROMPT_SELECT_CRT_PATH] == false) {
            return;
        }
        if (![self checkUSBHIDConnection]) {
            return;
        }
        // 事前に確認ダイアログを表示
        if ([ToolPopupWindow promptYesNo:MSG_INSTALL_SKEY_CERT
                         informativeText:MSG_PROMPT_INSTL_SKEY_CERT] == false) {
            return;
        }
        // 鍵・証明書インストール
        [self enableButtons:false];
        [[self toolHIDCommand] setInstallParameter:COMMAND_INSTALL_SKEY_CERT
                                      skeyFilePath:self.fieldPath1.stringValue
                                      certFilePath:self.fieldPath2.stringValue];
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_INSTALL_SKEY_CERT];
    }

    - (IBAction)button4DidPress:(id)sender {
        // PINコード設定画面を開く
        if (![self checkUSBHIDConnection]) {
            return;
        }
        [self enableButtons:false];
        [[self toolHIDCommand] setPinParamWindowWillOpen:self parentWindow:[self window]];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        return [[self toolHIDCommand] checkUSBHIDConnection];
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
        [[self toolFilePanel] prepareOpenPanel:MSG_BUTTON_SELECT
                                       message:MSG_PROMPT_SELECT_PKEY_PATH
                                     fileTypes:@[@"pem"]];
        [[self toolFilePanel] panelWillSelectPath:sender parentWindow:[self window]];
    }

    - (IBAction)buttonPath2DidPress:(id)sender {
        [self enableButtons:false];
        [[self toolFilePanel] prepareOpenPanel:MSG_BUTTON_SELECT
                                       message:MSG_PROMPT_SELECT_CRT_PATH
                                     fileTypes:@[@"crt"]];
        [[self toolFilePanel] panelWillSelectPath:sender parentWindow:[self window]];
    }

    - (IBAction)menuItemTestHID1DidSelect:(id)sender {
        // HID CTAP2ヘルスチェック実行
        [self performHealthCheckCommand:COMMAND_TEST_MAKE_CREDENTIAL];
    }

    - (IBAction)menuItemTestHID2DidSelect:(id)sender {
        // HID U2Fヘルスチェック実行
        [self performHealthCheckCommand:COMMAND_TEST_REGISTER];
    }

    - (IBAction)menuItemTestHID3DidSelect:(id)sender {
        if (![self checkUSBHIDConnection]) {
            return;
        }
        // PINGテスト実行
        [self enableButtons:false];
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TEST_CTAPHID_PING];
    }

    - (IBAction)menuItemTestHID4DidSelect:(id)sender {
        if (![self checkUSBHIDConnection]) {
            return;
        }
        // Flash ROM情報取得
        [self enableButtons:false];
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_HID_GET_FLASH_STAT];
    }

    - (IBAction)menuItemTestHID5DidSelect:(id)sender {
        if (![self checkUSBHIDConnection]) {
            return;
        }
        // バージョン情報取得
        [self enableButtons:false];
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_HID_GET_VERSION_INFO];
    }

    - (IBAction)menuItemTestBLE1DidSelect:(id)sender {
        // BLE CTAP2ヘルスチェック実行（PINコード入力画面を開く）
        [self enableButtons:false];
        [[self toolBLECommand] pinCodeParamWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)menuItemTestBLE2DidSelect:(id)sender {
        // BLE U2Fヘルスチェック実行
        [self enableButtons:false];
        [[self toolBLECommand] bleCommandWillProcess:COMMAND_TEST_REGISTER];
    }

    - (IBAction)menuItemTestBLE3DidSelect:(id)sender {
        // BLE PINGテスト実行
        [self enableButtons:false];
        [[self toolBLECommand] bleCommandWillProcess:COMMAND_TEST_BLE_PING];
    }

    - (IBAction)menuItemPreferencesDidSelect:(id)sender {
        // ツール設定画面を開く
        [self enableButtons:false];
        [[self toolPreferenceCommand] toolPreferenceWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)menuItemViewLogDidSelect:(id)sender {
        // ログファイル格納ディレクトリーをFinderで表示
        NSURL *url =
        [NSURL fileURLWithPath:[[ToolLogFile defaultLogger] logFilePathString] isDirectory:false];
        NSArray *fileURLs = [NSArray arrayWithObjects:url, nil];
        [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:fileURLs];
    }

    - (IBAction)menuItemDFUTestDidSelect:(id)sender {
        if ([self checkUSBHIDConnection]) {
            [self enableButtons:false];
            [[self toolDFUCommand] dfuProcessWillStart:self parentWindow:[self window] toolHIDCommandRef:[self toolHIDCommand]];
        }
    }

    - (IBAction)menuItemDFUNewDidSelect:(id)sender {
        [self enableButtons:false];
        [[self toolDFUCommand] dfuNewProcessWillStart:self parentWindow:[self window]];
    }

    - (IBAction)menuItemEraseBondDidSelect:(id)sender {
        if (![self checkUSBHIDConnection]) {
            return;
        }
        // 事前に確認ダイアログを表示
        if ([ToolPopupWindow promptYesNo:MSG_ERASE_BONDS
                         informativeText:MSG_PROMPT_ERASE_BONDS] == false) {
            return;
        }
        // ペアリング情報削除
        [self enableButtons:false];
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_ERASE_BONDS];
    }

    - (IBAction)menuItemBLModeDidSelect:(id)sender {
        if (![self checkUSBHIDConnection]) {
            return;
        }
        // 事前に確認ダイアログを表示
        if ([ToolPopupWindow promptYesNo:MSG_BOOT_LOADER_MODE
                         informativeText:MSG_PROMPT_BOOT_LOADER_MODE] == false) {
            return;
        }
        // ブートローダーモード遷移
        [self enableButtons:false];
        [self hidCommandStartedProcess:COMMAND_HID_BOOTLOADER_MODE];
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_HID_BOOTLOADER_MODE
                                           withData:nil forCommand:self];
    }

#pragma mark - Perform health check

    - (void)performHealthCheckCommand:(Command)command {
        // USBポートに接続されていない場合は終了
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 事前にツール設定照会を実行
        [self enableButtons:false];
        [self setHealthCheckCommand:command];
        [[self toolPreferenceCommand] toolPreferenceInquiryWillProcess];
    }

    - (void)toolPreferenceInquiryDidProcess:(Command)command
                                 CMD:(uint8_t)cmd response:(NSData *)resp
                              result:(bool)result message:(NSString *)message {
        // 処理失敗時は終了
        if (result == false) {
            [self commandDidProcess:COMMAND_NONE result:result message:message];
            return;
        }
        // ツール設定情報を共有情報に保持させる
        [[ToolContext instance] setBleScanAuthEnabled:[[self toolPreferenceCommand] bleScanAuthEnabled]];
        if ([[ToolContext instance] bleScanAuthEnabled]) {
            // ツール設定でBLE自動認証機能が有効化されている場合は確認メッセージを表示
            if ([ToolPopupWindow promptYesNo:MSG_PROMPT_START_HCHK_BLE_AUTH
                             informativeText:MSG_COMMENT_START_HCHK_BLE_AUTH] == false) {
                // メッセージダイアログでNOをクリックした場合は終了
                [self commandDidProcess:COMMAND_NONE result:true message:nil];
                return;
            }
        }
        switch ([self healthCheckCommand]) {
            case COMMAND_TEST_MAKE_CREDENTIAL:
                // HID CTAP2ヘルスチェック処理を実行（PINコード入力画面を開く）
                [[self toolHIDCommand] pinCodeParamWindowWillOpen:self parentWindow:[self window]];
                break;
            case COMMAND_TEST_REGISTER:
                // HID U2Fヘルスチェック処理を実行
                [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TEST_REGISTER];
                break;
            default:
                break;
        }
    }

#pragma mark - Interface for ToolPreferenceWindow

    - (void)toolPreferenceWillProcess:(Command)command withData:(NSData *)data {
        // コマンド実行のために必要なデータを設定し、コマンドを実行
        [[self toolHIDCommand] hidHelperWillProcess:command withData:data];
    }

    - (void)toolPreferenceDidProcess:(Command)command
                                 CMD:(uint8_t)cmd response:(NSData *)resp
                              result:(bool)result message:(NSString *)message {
        // ツール設定画面に応答メッセージを引き渡す
        [[self toolPreferenceCommand] toolPreferenceDidProcess:command
            CMD:cmd response:resp result:result message:message];
    }

    - (void)toolPreferenceWindowDidClose {
        // ツール設定画面を閉じた時は、ポップアップを表示しない
        [self commandDidProcess:COMMAND_NONE result:true message:nil];
    }

#pragma mark - Call back from ToolDFUCommand

    - (void)toolDFUCommandDidStart {
        // DFU処理開始時
        [self commandStartedProcess:COMMAND_USB_DFU type:TRANSPORT_HID];
    }

    - (void)toolDFUCommandDidTerminate:(Command)command result:(bool)result message:(NSString *)message {
        // DFU処理完了時
        [self commandDidProcess:command result:result message:message];
    }

#pragma mark - Call back from ToolFilePanel

    - (void)panelDidSelectPath:(id)sender filePath:(NSString*)filePath
                 modalResponse:(NSInteger)modalResponse {
        // OKボタン押下時は、ファイル選択パネルで選択されたファイルパスを表示する
        if (modalResponse == NSFileHandlingPanelOKButton) {
            if ([self buttonPath1] == sender) {
                [[self fieldPath1] setStringValue:filePath];
                [[self fieldPath1] becomeFirstResponder];
            }
            if ([self buttonPath2] == sender) {
                [[self fieldPath2] setStringValue:filePath];
                [[self fieldPath2] becomeFirstResponder];
            }
        }
        // メニューを活性化
        [self enableButtons:true];
    }

#pragma mark - Call back from ToolCommand

    - (void)notifyToolCommandMessage:(NSString *)message {
        // 画面上のテキストエリアにメッセージを表示する
        if (message) {
            [self appendLogMessage:message];
        }
    }

    - (void)bleCommandDidProcess:(Command)command
                          result:(bool)result message:(NSString *)message {
        [self commandDidProcess:command result:result message:message];
    }

    - (void)bleCommandStartedProcess:(Command)command {
        [self commandStartedProcess:command type:TRANSPORT_BLE];
    }

#pragma mark - Call back from ToolHIDCommand

    - (void)hidCommandDidProcess:(Command)command
                             CMD:(uint8_t)cmd response:(NSData *)resp
                          result:(bool)result message:(NSString *)message {
        switch (command) {
            case COMMAND_TOOL_PREF_PARAM:
            case COMMAND_TOOL_PREF_PARAM_INQUIRY:
                // ツール設定コマンドに応答メッセージを引き渡す
                [self toolPreferenceDidProcess:command
                        CMD:cmd response:resp result:result message:message];
                break;
            default:
                [self commandDidProcess:command result:result message:message];
                break;
        }
    }

    - (void)hidCommandStartedProcess:(Command)command {
        [self commandStartedProcess:command type:TRANSPORT_HID];
    }

    - (void)hidCommandDidDetectConnect {
        [self notifyToolCommandMessage:MSG_HID_CONNECTED];
        [[ToolLogFile defaultLogger] info:MSG_HID_CONNECTED];
        // DFU処理にHID接続開始を通知
        [[self toolDFUCommand] hidCommandDidDetectConnect:[self toolHIDCommand]];
    }

    - (void)hidCommandDidDetectRemoval {
        [self notifyToolCommandMessage:MSG_HID_REMOVED];
        [[ToolLogFile defaultLogger] info:MSG_HID_REMOVED];
        // DFU処理にHID接続切断を通知
        [[self toolDFUCommand] hidCommandDidDetectRemoval:[self toolHIDCommand]];
    }

#pragma mark - Common method called by callback

    - (void)commandStartedProcess:(Command)command type:(TransportType)type {
        // コマンド種別に対応する処理名称を設定
        [self setProcessNameOfCommand:nil];
        switch (command) {
            // BLE関連
            case COMMAND_PAIRING:
                [self setProcessNameOfCommand:PROCESS_NAME_PAIRING];
                break;
            case COMMAND_TEST_BLE_PING:
                [self setProcessNameOfCommand:PROCESS_NAME_TEST_BLE_PING];
                break;
            // HID関連
            case COMMAND_ERASE_BONDS:
                [self setProcessNameOfCommand:PROCESS_NAME_ERASE_BONDS];
                break;
            case COMMAND_ERASE_SKEY_CERT:
                [self setProcessNameOfCommand:PROCESS_NAME_ERASE_SKEY_CERT];
                break;
            case COMMAND_INSTALL_SKEY_CERT:
                [self setProcessNameOfCommand:PROCESS_NAME_INSTALL_SKEY_CERT];
                break;
            case COMMAND_TEST_CTAPHID_PING:
                [self setProcessNameOfCommand:PROCESS_NAME_TEST_CTAPHID_PING];
                break;
            case COMMAND_HID_GET_FLASH_STAT:
                [self setProcessNameOfCommand:PROCESS_NAME_GET_FLASH_STAT];
                break;
            case COMMAND_HID_GET_VERSION_INFO:
                [self setProcessNameOfCommand:PROCESS_NAME_GET_VERSION_INFO];
                break;
            case COMMAND_HID_BOOTLOADER_MODE:
                [self setProcessNameOfCommand:PROCESS_NAME_BOOT_LOADER_MODE];
                break;
            case COMMAND_CLIENT_PIN_SET:
                [self setProcessNameOfCommand:PROCESS_NAME_CLIENT_PIN_SET];
                break;
            case COMMAND_USB_DFU:
                [self setProcessNameOfCommand:PROCESS_NAME_USB_DFU];
                break;
            case COMMAND_CLIENT_PIN_CHANGE:
                [self setProcessNameOfCommand:PROCESS_NAME_CLIENT_PIN_CHANGE];
                break;
            case COMMAND_AUTH_RESET:
                [self setProcessNameOfCommand:PROCESS_NAME_AUTH_RESET];
                break;
            // BLE、HID共通
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                if (type == TRANSPORT_BLE) {
                    [self setProcessNameOfCommand:PROCESS_NAME_BLE_CTAP2_HEALTHCHECK];
                }
                if (type == TRANSPORT_HID) {
                    [self setProcessNameOfCommand:PROCESS_NAME_HID_CTAP2_HEALTHCHECK];
                }
                break;
            case COMMAND_TEST_REGISTER:
            case COMMAND_TEST_AUTH_CHECK:
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                if (type == TRANSPORT_BLE) {
                    [self setProcessNameOfCommand:PROCESS_NAME_BLE_U2F_HEALTHCHECK];
                }
                if (type == TRANSPORT_HID) {
                    [self setProcessNameOfCommand:PROCESS_NAME_HID_U2F_HEALTHCHECK];
                }
                break;
            default:
                break;
        }
        if ([self processNameOfCommand]) {
            // コマンド開始メッセージを画面表示し、ログファイルにも出力
            NSString *startMsg = [NSString stringWithFormat:MSG_FORMAT_START_MESSAGE,
                                  [self processNameOfCommand]];
            [self notifyToolCommandMessage:startMsg];
            [[ToolLogFile defaultLogger] info:startMsg];
        }
    }

    - (void)commandDidProcess:(Command)command result:(bool)result message:(NSString *)message {
        // 処理失敗時は、引数に格納されたエラーメッセージを画面出力
        if (result == false) {
            [self notifyToolCommandMessage:message];
        }
        // コマンド名称を取得
        if (command != COMMAND_NONE) {
            // テキストエリアとポップアップの両方に表示させる処理終了メッセージを作成
            NSString *str = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE,
                             [self processNameOfCommand],
                             result? MSG_SUCCESS:MSG_FAILURE];
            // メッセージを画面のテキストエリアに表示
            [self notifyToolCommandMessage:str];
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
