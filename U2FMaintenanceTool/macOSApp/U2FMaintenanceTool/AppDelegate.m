#import "AppDelegate.h"
#import "ToolBLECentral.h"
#import "ToolHIDCommand.h"
#import "ToolCommand.h"
#import "ToolFileMenu.h"
#import "ToolFilePanel.h"
#import "ToolParamWindow.h"
#import "ToolPopupWindow.h"
#import "ToolCommonMessage.h"

@interface AppDelegate ()
    <ToolBLECentralDelegate, ToolHIDCommandDelegate, ToolCommandDelegate, ToolFileMenuDelegate, ToolFilePanelDelegate>

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

    @property (assign) IBOutlet NSMenuItem  *menuItemFile1;
    @property (assign) IBOutlet NSMenuItem  *menuItemFile2;
    @property (assign) IBOutlet NSMenuItem  *menuItemFile3;

    @property (assign) IBOutlet NSMenuItem  *menuItemTestUSB;
    @property (assign) IBOutlet NSMenuItem  *menuItemTestBLE;

    @property (nonatomic) ToolCommand       *toolCommand;
    @property (nonatomic) ToolBLECentral    *toolBLECentral;
    @property (nonatomic) ToolHIDCommand    *toolHIDCommand;
    @property (nonatomic) ToolFileMenu      *toolFileMenu;
    @property (nonatomic) ToolFilePanel     *toolFilePanel;

    @property (nonatomic) NSUInteger         bleConnectionRetryCount;
    @property (nonatomic) bool               bleTransactionStarted;

    @property (nonatomic) NSString          *lastCommandMessage;
    @property (nonatomic) bool               lastCommandSuccess;

@end

@implementation AppDelegate

    - (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
        self.toolBLECentral = [[ToolBLECentral alloc] initWithDelegate:self];
        self.toolHIDCommand = [[ToolHIDCommand alloc]  initWithDelegate:self];
        self.toolCommand    = [[ToolCommand alloc]    initWithDelegate:self];
        self.toolFileMenu   = [[ToolFileMenu alloc]   initWithDelegate:self];
        self.toolFilePanel  = [[ToolFilePanel alloc]  initWithDelegate:self];

        self.textView.font = [NSFont fontWithName:@"Courier" size:12];
    }

    - (void)applicationWillTerminate:(NSNotification *)notification {
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
        [self.menuItemFile1 setEnabled:enabled];
        [self.menuItemFile2 setEnabled:enabled];
        [self.menuItemFile3 setEnabled:enabled];
        [self.menuItemTestUSB setEnabled:enabled];
        [self.menuItemTestBLE setEnabled:enabled];
    }

    - (IBAction)button1DidPress:(id)sender {
        // ペアリング実行
        [self enableButtons:false];
        [self.toolCommand toolCommandWillCreateBleRequest:COMMAND_PAIRING];
    }

    - (IBAction)button2DidPress:(id)sender {
        // 鍵・証明書削除
        if ([ToolPopupWindow promptYesNo:MSG_ERASE_SKEY_CERT
                         informativeText:MSG_PROMPT_ERASE_SKEY_CERT] == false) {
            return;
        }
        [self enableButtons:false];
        [self.toolCommand toolCommandWillCreateBleRequest:COMMAND_ERASE_SKEY_CERT];
    }

    - (bool)checkPathEntry:(NSTextField *)field messageIfError:(NSString *)message {
        // 入力項目が正しく指定されていない場合は終了
        if ([ToolParamWindow checkMustEntry:field informativeText:message] == false) {
            return false;
        }
        // 入力されたファイルパスが存在しない場合は終了
        if ([ToolParamWindow checkFileExist:field informativeText:message] == false) {
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
        // 鍵・証明書インストール
        [self enableButtons:false];
        [self.toolCommand setInstallParameter:COMMAND_INSTALL_SKEY
                            skeyFilePath:self.fieldPath1.stringValue
                            certFilePath:self.fieldPath2.stringValue];
        [self.toolCommand toolCommandWillCreateBleRequest:COMMAND_INSTALL_SKEY];
    }

    - (IBAction)button4DidPress:(id)sender {
        // ヘルスチェック実行
        [self enableButtons:false];
        [self.toolCommand toolCommandWillCreateBleRequest:COMMAND_TEST_REGISTER];
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
                                       message:MSG_PROMPT_SELECT_PEM_PATH
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

    - (IBAction)menuItemFile1DidSelect:(id)sender {
        [self enableButtons:false];
        [[self toolFileMenu] toolFileMenuWillCreateFile:self parentWindow:[self window]
                                                command:COMMAND_CREATE_KEYPAIR_PEM];
    }

    - (IBAction)menuItemFile2DidSelect:(id)sender {
        [self enableButtons:false];
        [[self toolFileMenu] toolFileMenuWillCreateFile:self parentWindow:[self window]
                                                command:COMMAND_CREATE_CERTREQ_CSR];
    }

    - (IBAction)menuItemFile3DidSelect:(id)sender {
        [self enableButtons:false];
        [[self toolFileMenu] toolFileMenuWillCreateFile:self parentWindow:[self window]
                                                command:COMMAND_CREATE_SELFCRT_CRT];
    }

    - (IBAction)menuItemTestHID1DidSelect:(id)sender {
        [self enableButtons:false];
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TEST_CTAPHID_INIT];
    }

    - (IBAction)menuItemTestBLE1DidSelect:(id)sender {
        [self enableButtons:false];
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_NONE];
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

    - (void)panelDidCreatePath:(id)sender filePath:(NSString*)filePath
                 modalResponse:(NSInteger)modalResponse {
    }

#pragma mark - Call back from ToolCommand

    - (void)notifyToolCommandMessage:(NSString *)message {
        // 画面上のテキストエリアにメッセージを表示する
        [self appendLogMessage:message];
    }

    - (void)toolCommandDidCreateBleRequest {
        // 再試行回数をゼロクリアし、BLEデバイス接続処理に移る
        [self setBleConnectionRetryCount:0];
        [self startBleConnection];
    }

    - (void)startBleConnection {
        // メッセージ表示用変数を初期化
        [self setLastCommandMessage:nil];
        [self setLastCommandSuccess:false];
        // BLEデバイス接続処理を開始する
        [self setBleTransactionStarted:false];
        [[self toolBLECentral] centralManagerWillConnect];
    }

    - (void)toolCommandDidReceive:(Command)command result:(bool)result {
        // デバイス接続を切断
        [[self toolBLECentral] centralManagerWillDisconnect];
    }

    - (void)toolCommandDidProcess:(Command)command result:(bool)result
                          message:(NSString *)message {
        // 処理失敗時は、引数に格納されたエラーメッセージを画面出力
        if (result == false) {
            [self notifyToolCommandMessage:message];
        }
        // テキストエリアとポップアップの両方に表示させる処理終了メッセージを作成
        NSString *str = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE,
                         [ToolCommon processNameOfCommand:command],
                         result? MSG_SUCCESS:MSG_FAILURE];
        // 処理終了メッセージとリザルトを保持
        [self setLastCommandMessage:str];
        [self setLastCommandSuccess:result];
        // デバイス接続を切断
        [[self toolBLECentral] centralManagerWillDisconnect];
    }

#pragma mark - Call back from ToolFileMenu

    - (void)notifyToolFileMenuMessage:(NSString *)message {
        // 画面上のテキストエリアにメッセージを表示する
        [self appendLogMessage:message];
    }

    - (void)notifyToolFileMenuEnd {
        // ボタンを活性化
        [self enableButtons:true];
    }

#pragma mark - Call back from ToolBLECentral

    - (void)notifyCentralManagerStateUpdate:(CBCentralManagerState)state {
        NSLog(@"centralManagerDidUpdateState: %ld", state);
    }

    - (void)centralManagerDidConnect {
        // U2F Control Pointに実行コマンドを書込
        [self.toolBLECentral centralManagerWillSend:[self.toolCommand bleRequestArray]];
        [self setBleTransactionStarted:true];
    }

    - (void)centralManagerDidFailConnectionWith:(NSString *)message error:(NSError *)error {
        // コンソールにエラーメッセージを出力
        [self displayErrorMessage:message error:error];
        
        // 画面上のテキストエリアにもメッセージを表示する
        [self appendLogMessage:message];

        // トランザクション完了済とし、接続再試行を回避
        [self setBleTransactionStarted:false];
        // ポップアップ表示させる失敗メッセージとリザルトを保持
        [self setLastCommandMessage:MSG_OCCUR_BLECONN_ERROR];
        [self setLastCommandSuccess:false];
        // デバイス接続を切断
        [[self toolBLECentral] centralManagerWillDisconnect];
    }

    - (void)centralManagerDidDisconnectWith:(NSString *)message error:(NSError *)error {
        // コンソールにエラーメッセージを出力
        [self displayErrorMessage:message error:error];
        
        // トランザクション実行中に切断された場合は、接続を再試行（回数上限あり）
        if ([self retryBLEConnection]) {
            return;
        }
        
        // ボタンを活性化し、ポップアップメッセージを表示
        [self terminateProcessOnWindow];
    }

    - (void)terminateProcessOnWindow {
        // ボタンを活性化
        [self enableButtons:true];
        // メッセージが設定されていない場合は何もしない
        if ([self lastCommandMessage] == nil || [[self lastCommandMessage] length] == 0) {
            return;
        }
        // メッセージを画面のテキストエリアに表示
        [self notifyToolCommandMessage:[self lastCommandMessage]];
        // ポップアップを表示
        if ([self lastCommandSuccess]) {
            [ToolPopupWindow informational:[self lastCommandMessage] informativeText:nil];
        } else {
            [ToolPopupWindow critical:[self lastCommandMessage] informativeText:nil];
        }
    }

    - (bool)retryBLEConnection {
        // 処理が開始されていない場合はfalseを戻す
        if ([self bleTransactionStarted] == false) {
            return false;
        }
        
        if ([self bleConnectionRetryCount] < BLE_CONNECTION_RETRY_MAX_COUNT) {
            // 再試行回数をカウントアップ
            [self setBleConnectionRetryCount:([self bleConnectionRetryCount] + 1)];
            NSLog(MSG_BLE_CONNECTION_RETRY_WITH_CNT,
                  (unsigned long)[self bleConnectionRetryCount]);
            // BLEデバイス接続処理に移る
            [self startBleConnection];
            return true;
            
        } else {
            // 再試行上限回数に達している場合は、その旨コンソールログに出力
            NSLog(MSG_BLE_CONNECTION_RETRY_END);
            // ポップアップ表示させる失敗メッセージとリザルトを保持
            [self setLastCommandMessage: MSG_BLE_CONNECTION_RETRY_END];
            [self setLastCommandSuccess:false];
            return false;
        }
    }

    - (void)notifyCentralManagerMessage:(NSString *)message {
        if (message == nil) {
            return;
        }
        // コンソールログを出力
        NSLog(@"%@", message);
    }

    - (void)displayErrorMessage:(NSString *)message error:(NSError *)error {
        if (message == nil) {
            return;
        }
        // コンソールログを出力
        if (error) {
            NSLog(@"%@ %@", message, [error description]);
        } else {
            NSLog(@"%@", message);
        }
    }

    - (void)centralManagerDidReceive:(NSData *)bleResponse {
        if ([self.toolCommand isResponseCompleted:bleResponse]) {
            // 後続レスポンスがあれば、タイムアウト監視を再開させ、後続レスポンスを待つ
            [self.toolBLECentral centralManagerWillStartResponseTimeout];
        } else {
            // 後続レスポンスがなければ、トランザクション完了と判断
            [self setBleTransactionStarted:false];
            // レスポンスを次処理に引き渡す
            [self.toolCommand toolCommandWillProcessBleResponse];
        }
    }

#pragma mark - Call back from ToolHIDCommand

    - (void)hidCommandDidProcess:(bool)success result:(bool)result message:(NSString *)message {
        // ボタンを活性化
        [self enableButtons:true];
        // メッセージが設定されていない場合は何もしない
        if (message == nil || [message length] == 0) {
            return;
        }
        // メッセージを画面のテキストエリアに表示
        [self notifyToolCommandMessage:message];
        // ポップアップを表示
        if (success) {
            [ToolPopupWindow informational:message informativeText:nil];
        } else {
            [ToolPopupWindow critical:message informativeText:nil];
        }
    }

@end
