#import "AppDelegate.h"
#import "ToolBLECentral.h"
#import "ToolBLEHelper.h"
#import "ToolCommand.h"
#import "ToolFileMenu.h"
#import "ToolFilePanel.h"
#import "ToolParamWindow.h"
#import "ToolPopupWindow.h"
#import "ToolCommonMessage.h"

@interface AppDelegate ()
    <ToolBLECentralDelegate, ToolBLEHelperDelegate, ToolCommandDelegate, ToolFileMenuDelegate, ToolFilePanelDelegate>

    @property (assign) IBOutlet NSWindow   *window;
    @property (assign) IBOutlet NSButton   *button1;
    @property (assign) IBOutlet NSButton   *button2;
    @property (assign) IBOutlet NSButton   *button3;
    @property (assign) IBOutlet NSButton   *button4;
    @property (assign) IBOutlet NSButton   *button5;
    @property (assign) IBOutlet NSButton   *buttonQuit;
    @property (assign) IBOutlet NSTextView *textView;

    @property (assign) IBOutlet NSTextField *fieldPath1;
    @property (assign) IBOutlet NSTextField *fieldPath2;
    @property (assign) IBOutlet NSButton    *buttonPath1;
    @property (assign) IBOutlet NSButton    *buttonPath2;

    @property (assign) IBOutlet NSMenuItem  *menuItemFile1;
    @property (assign) IBOutlet NSMenuItem  *menuItemFile2;
    @property (assign) IBOutlet NSMenuItem  *menuItemFile3;

    @property (nonatomic) ToolCommand       *toolCommand;
    @property (nonatomic) ToolBLECentral    *toolBLECentral;
    @property (nonatomic) ToolBLEHelper     *toolBLEHelper;
    @property (nonatomic) ToolFileMenu      *toolFileMenu;
    @property (nonatomic) ToolFilePanel     *toolFilePanel;

@end

@implementation AppDelegate

    - (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
        self.toolBLECentral = [[ToolBLECentral alloc] initWithDelegate:self];
        self.toolBLEHelper  = [[ToolBLEHelper alloc]  initWithDelegate:self];
        self.toolCommand    = [[ToolCommand alloc]    initWithDelegate:self];
        self.toolFileMenu   = [[ToolFileMenu alloc]   initWithDelegate:self];
        self.toolFilePanel  = [[ToolFilePanel alloc]  initWithDelegate:self];

        self.textView.font = [NSFont fontWithName:@"Courier" size:12];
        
        // Chromeエクステンションから起動した時はボタンを押下不可とする
        if ([self.toolBLEHelper bleHelperCommunicateAsChromeNative]) {
            [self enableButtons:false];
        }
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
        [self.button5 setEnabled:enabled];
        [self.fieldPath1 setEnabled:enabled];
        [self.fieldPath2 setEnabled:enabled];
        [self.buttonPath1 setEnabled:enabled];
        [self.buttonPath2 setEnabled:enabled];
        [self.buttonQuit setEnabled:enabled];
        [self.menuItemFile1 setEnabled:enabled];
        [self.menuItemFile2 setEnabled:enabled];
        [self.menuItemFile3 setEnabled:enabled];
    }

    - (IBAction)button1DidPress:(id)sender {
        // ペアリング情報削除
        [self enableButtons:false];
        [self.toolCommand toolCommandWillCreateBleRequest:COMMAND_ERASE_BOND];
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

    - (IBAction)button5DidPress:(id)sender {
        if ([ToolPopupWindow promptYesNo:MSG_SETUP_CHROME
                         informativeText:MSG_PROMPT_SETUP_CHROME] == false) {
            return;
        }
        // Chrome Native Messaging有効化設定
        [self enableButtons:false];
        [self.toolCommand toolCommandWillSetup:COMMAND_SETUP_CHROME_NATIVE_MESSAGING];
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

    - (void)toolCommandDidCreateBleRequest {
        // BLEデバイス接続処理に移る
        [self.toolBLECentral centralManagerWillConnect];
    }

    - (void)toolCommandDidReceive:(NSDictionary *)u2fResponseDict {
        // U2F処理実行結果をChromeエクステンションに戻す
        [self.toolBLEHelper bleHelperWillSend:u2fResponseDict];
    }

    - (void)bleHelperDidSend:(NSData *)chromeMessageData {
        // デバイス接続を切断
        if (chromeMessageData) {
            NSLog(@"Sent response to chrome: %@", chromeMessageData);
        }
        [self.toolBLECentral centralManagerWillDisconnect];
    }

    - (void)notifyToolCommandMessage:(NSString *)message {
        // 画面上のテキストエリアにメッセージを表示する
        [self appendLogMessage:message];
    }

    - (void)notifyToolCommandEnd {
        // デバイス接続を切断
        [self.toolBLECentral centralManagerWillDisconnect];
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

        if (state == CBCentralManagerStatePoweredOn) {
            // BLEデバイスが有効化された後に
            // Chromeエクステンションからのメッセージ受信を有効化
            [self.toolBLEHelper bleHelperWillSetStdinNotification];
        }
    }

    - (void)centralManagerDidConnect {
        // U2F Control Pointに実行コマンドを書込
        [self.toolBLECentral centralManagerWillSend:[self.toolCommand bleRequestArray]];
    }

    - (void)centralManagerDidFailConnection {
        // 画面上のテキストエリアにメッセージを表示する
        [self appendLogMessage:MSG_OCCUR_BLECONN_ERROR];
        // 失敗メッセージを表示
        [ToolPopupWindow critical:MSG_OCCUR_BLECONN_ERROR informativeText:nil];
        // デバイス接続を切断
        [self.toolBLECentral centralManagerWillDisconnect];
    }

    - (void)centralManagerDidDisconnect {
        if ([[self toolCommand] command] == COMMAND_U2F_PROCESS) {
            // Chrome native messaging時は、このアプリケーションを終了させる
            NSLog(@"Chrome native messaging host will terminate");
            [NSApp terminate:self];
        } else {
            // ボタンを活性化
            [self enableButtons:true];
        }
    }

    - (void)notifyCentralManagerMessage:(NSString *)message {
        if (message == nil) {
            return;
        }
        // コンソールログを出力
        NSLog(@"%@", message);
        // 画面上のテキストエリアにメッセージを表示する
        [self appendLogMessage:message];
    }

    - (void)notifyCentralManagerErrorMessage:(NSString *)message error:(NSError *)error {
        if (message == nil) {
            return;
        }
        // コンソールログを出力
        if (error) {
            NSLog(@"%@ %@", message, [error description]);
        } else {
            NSLog(@"%@", message);
        }
        // 画面上のテキストエリアにメッセージを表示する
        [self appendLogMessage:message];
    }

    - (void)centralManagerDidReceive:(NSData *)bleResponse {
        if ([self.toolCommand isResponseCompleted:bleResponse]) {
            // 後続レスポンスがあれば、タイムアウト監視を再開させ、後続レスポンスを待つ
            [self.toolBLECentral centralManagerWillStartResponseTimeout];
        } else {
            // 後続レスポンスがなければ、レスポンスを次処理に引き渡す
            [self.toolCommand toolCommandWillProcessBleResponse];
        }
    }

#pragma mark - Call back from ToolBLEHelper

    - (void)bleHelperDidReceive:(NSArray<NSDictionary *> *)bleHelperMessages {
        // Chromeエクステンションからの受信データによりU2F処理を実行
        [self.toolCommand setU2FProcessParameter:COMMAND_U2F_PROCESS
                               bleHelperMessages:bleHelperMessages];
        [self.toolCommand toolCommandWillCreateBleRequest:COMMAND_U2F_PROCESS];
    }

@end
