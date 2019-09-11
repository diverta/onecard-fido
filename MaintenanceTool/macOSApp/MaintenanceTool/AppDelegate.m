#import "AppDelegate.h"
#import "ToolHIDCommand.h"
#import "ToolBLECommand.h"
#import "ToolFilePanel.h"
#import "ToolPopupWindow.h"
#import "ToolCommonMessage.h"

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

    @property (nonatomic) ToolBLECommand    *toolBLECommand;
    @property (nonatomic) ToolHIDCommand    *toolHIDCommand;
    @property (nonatomic) ToolFilePanel     *toolFilePanel;

@end

@implementation AppDelegate

    - (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
        self.toolHIDCommand = [[ToolHIDCommand alloc]  initWithDelegate:self];
        self.toolBLECommand = [[ToolBLECommand alloc]  initWithDelegate:self];
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
        [self.menuItemTestUSB setEnabled:enabled];
        [self.menuItemTestBLE setEnabled:enabled];
    }

    - (IBAction)button1DidPress:(id)sender {
        // ペアリング実行
        [self enableButtons:false];
        [[self toolBLECommand] bleCommandWillProcess:COMMAND_PAIRING];
    }

    - (IBAction)button2DidPress:(id)sender {
        if (![[self toolHIDCommand] checkUSBHIDConnection]) {
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
        if (![[self toolHIDCommand] checkUSBHIDConnection]) {
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
        if (![[self toolHIDCommand] checkUSBHIDConnection]) {
            return;
        }
        [self enableButtons:false];
        [[self toolHIDCommand] setPinParamWindowWillOpen:self parentWindow:[self window]];
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
        // PINコード入力画面を開く
        if (![[self toolHIDCommand] checkUSBHIDConnection]) {
            return;
        }
        [self enableButtons:false];
        [[self toolHIDCommand] pinCodeParamWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)menuItemTestHID2DidSelect:(id)sender {
        if (![[self toolHIDCommand] checkUSBHIDConnection]) {
            return;
        }
        // PINGテスト実行
        [self enableButtons:false];
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TEST_CTAPHID_PING];
    }

    - (IBAction)menuItemTestHID3DidSelect:(id)sender {
        if (![[self toolHIDCommand] checkUSBHIDConnection]) {
            return;
        }
        // Flash ROM情報取得
        [self enableButtons:false];
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_HID_GET_FLASH_STAT];
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
        if (message) {
            [self appendLogMessage:message];
        }
    }

    - (void)bleCommandDidProcess:(NSString *)processNameOfCommand
                          result:(bool)result message:(NSString *)message {
        [self commandDidProcess:processNameOfCommand result:result message:message];
    }

#pragma mark - Call back from ToolHIDCommand

    - (void)hidCommandDidProcess:(NSString *)processNameOfCommand
                          result:(bool)result message:(NSString *)message {
        [self commandDidProcess:processNameOfCommand result:result message:message];
    }

#pragma mark - Common method called by callback

    - (void)commandDidProcess:(NSString *)processNameOfCommand result:(bool)result message:(NSString *)message {
        // 処理失敗時は、引数に格納されたエラーメッセージを画面出力
        if (result == false) {
            [self notifyToolCommandMessage:message];
        }
        // コマンド名称を取得
        if (processNameOfCommand) {
            // テキストエリアとポップアップの両方に表示させる処理終了メッセージを作成
            NSString *str = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE,
                             processNameOfCommand,
                             result? MSG_SUCCESS:MSG_FAILURE];
            // メッセージを画面のテキストエリアに表示
            [self notifyToolCommandMessage:str];
            // ポップアップを表示
            if (result) {
                [ToolPopupWindow informational:str informativeText:nil];
            } else {
                [ToolPopupWindow critical:str informativeText:nil];
            }
        }
        // ボタンを活性化
        [self enableButtons:true];
    }

@end
