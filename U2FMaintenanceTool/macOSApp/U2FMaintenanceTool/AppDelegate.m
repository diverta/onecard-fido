#import "AppDelegate.h"
#import "ToolBLECentral.h"
#import "ToolBLEHelper.h"
#import "ToolCommand.h"

typedef enum : NSInteger {
    PATH_SKEY = 1,
    PATH_CERT,
    PATH_CSR
} PathType;

@interface AppDelegate ()
    <ToolBLECentralDelegate, ToolBLEHelperDelegate, ToolCommandDelegate>

    @property (nonatomic) ToolCommand    *toolCommand;
    @property (nonatomic) ToolBLECentral *toolBLECentral;
    @property (nonatomic) ToolBLEHelper  *toolBLEHelper;

    @property (nonatomic) PathType  pathType;

@end

@implementation AppDelegate

    - (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
        self.toolBLECentral = [[ToolBLECentral alloc] initWithDelegate:self];
        self.toolBLEHelper  = [[ToolBLEHelper alloc]  initWithDelegate:self];
        self.toolCommand    = [[ToolCommand alloc]    initWithDelegate:self];

        self.textView.font = [NSFont fontWithName:@"Courier" size:12];
        
        // Chromeエクステンションから起動した時はボタンを押下不可とする
        if ([self.toolBLEHelper bleHelperCommunicateAsChromeNative]) {
            [self enableButtons:false];
        }
    }

    - (void)applicationWillTerminate:(NSNotification *)notification {
        [self.toolBLECentral centralManagerWillDisconnect];
    }

    - (void)appendLogMessage:(NSString *)message {
        // テキストフィールドにメッセージを追加し、末尾に移動
        self.textView.string = [self.textView.string stringByAppendingFormat:@"%@\n", message];
        [self.textView performSelector:@selector(scrollToEndOfDocument:) withObject:nil afterDelay:0];
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
    }

    - (IBAction)button1DidPress:(id)sender {
        // ペアリング情報削除
        [self enableButtons:false];
        [self.toolCommand toolCommandWillCreateBleRequest:COMMAND_ERASE_BOND];
    }

    - (IBAction)button2DidPress:(id)sender {
        // 鍵・証明書削除
        [self enableButtons:false];
        [self.toolCommand toolCommandWillCreateBleRequest:COMMAND_ERASE_SKEY_CERT];
    }

    - (bool)checkPathEntry:(NSTextField *)field messageIfError:(NSString *)message {
        // 入力済みの場合はチェックOK
        if ([field.stringValue length]) {
            return true;
        }
        // 未入力の場合はポップアップメッセージを表示
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setAlertStyle:NSAlertStyleWarning];
        [alert setMessageText:message];
        [alert runModal];
        [field becomeFirstResponder];
        return false;
    }

    - (IBAction)button3DidPress:(id)sender {
        if ([self checkPathEntry:self.fieldPath1 messageIfError:@"鍵ファイルのパスを選択してください"] == false) {
            return;
        }
        if ([self checkPathEntry:self.fieldPath2 messageIfError:@"証明書ファイルのパスを選択してください"] == false) {
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
        if ([self displayPromptPopup:@"ChromeでBLE U2Fトークンが使用できるよう設定します。"
                         informative:@"ChromeでBLE U2Fトークンを使用時、このU2F管理ツールがChromeのサブプロセスとして起動します。\n設定を実行しますか？"]) {
            // Chrome Native Messaging有効化設定
            [self enableButtons:false];
            [self.toolCommand toolCommandWillSetup:COMMAND_SETUP_CHROME_NATIVE_MESSAGING];
        }
    }

    - (bool)displayPromptPopup:(NSString *)prompt informative:(NSString *)informative {
        if (!prompt) {
            return false;
        }
        // ダイアログを作成
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setAlertStyle:NSAlertStyleInformational];
        [alert setMessageText:prompt];
        [alert setInformativeText:informative];
        [alert addButtonWithTitle:@"Yes"];
        [alert addButtonWithTitle:@"No"];
        // ダイアログを表示しYesボタンクリックを判定
        return ([alert runModal] == NSAlertFirstButtonReturn);
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
        [self buttonPathDidPress:sender pathType:PATH_SKEY];
    }

    - (IBAction)buttonPath2DidPress:(id)sender {
        [self buttonPathDidPress:sender pathType:PATH_CERT];
    }

    - (void)buttonPathDidPress:(id)sender pathType:(PathType)pathType {
        // ファイル選択パネルをモーダル表示
        [self setPathType:pathType];
        [self panelWillSelectPath:[self preparePanelForSelectPath]];
    }

    - (NSOpenPanel *)preparePanelForSelectPath {
        // ファイル選択パネルの設定
        NSOpenPanel *panel = [NSOpenPanel openPanel];
        [panel setAllowsMultipleSelection:NO];
        [panel setCanChooseDirectories:NO];
        [panel setCanChooseFiles:YES];
        [panel setResolvesAliases:NO];
        
        // 鍵=pem、証明書=crtのみ指定可能とする
        if (self.pathType == PATH_SKEY) {
            [panel setMessage:@"秘密鍵ファイル(PEM)を選択してください"];
            [panel setAllowedFileTypes:@[@"pem"]];
        } else {
            [panel setMessage:@"証明書ファイル(CRT)を選択してください"];
            [panel setAllowedFileTypes:@[@"crt"]];
        }
        [panel setPrompt:@"選択"];
        
        return panel;
    }

    - (void)panelWillSelectPath:(NSOpenPanel *)panel {
        // ファイル選択パネルをモーダル表示
        AppDelegate * __weak weakSelf = self;
        [panel beginSheetModalForWindow:self.window completionHandler:^(NSInteger result) {
            [panel orderOut:self];
            if (result != NSFileHandlingPanelOKButton) {
                return;
            }
            // ファイルが選択された時の処理
            [weakSelf panelDidSelectPath:panel];
        }];
    }

    - (void)panelDidSelectPath:(NSOpenPanel *)panel {
        // ファイル選択パネルで選択されたファイルパスを取得
        NSURL *url = [[panel URLs] objectAtIndex:0];
        NSString *filePath = [url path];
        // ファイル選択パネルで選択されたファイルパスを表示する
        if ([self pathType] == PATH_SKEY) {
            [self.fieldPath1 setStringValue:filePath];
            [self.fieldPath1 becomeFirstResponder];
        } else {
            [self.fieldPath2 setStringValue:filePath];
            [self.fieldPath2 becomeFirstResponder];
        }
    }

    - (IBAction)menuItemFile1DidSelect:(id)sender {
        // ファイル保存パネルをモーダル表示
        [self setPathType:PATH_SKEY];
        [self panelWillCreatePath:[self preparePanelForCreatePath]];
    }

    - (IBAction)menuItemFile2DidSelect:(id)sender {
        // ファイル保存パネルをモーダル表示
        [self setPathType:PATH_CSR];
        [self panelWillCreatePath:[self preparePanelForCreatePath]];
    }

    - (IBAction)menuItemFile3DidSelect:(id)sender {
        // ファイル保存パネルをモーダル表示
        [self setPathType:PATH_CERT];
        [self panelWillCreatePath:[self preparePanelForCreatePath]];
    }

    - (NSSavePanel *)preparePanelForCreatePath {
        // ファイル保存パネルの設定
        NSSavePanel *panel = [NSSavePanel savePanel];
        [panel setCanCreateDirectories:NO];
        [panel setShowsTagField:NO];
        // 鍵=pem、証明書=crtのみ指定可能とする
        if (self.pathType == PATH_SKEY) {
            [panel setMessage:@"作成する秘密鍵ファイル(PEM)名を指定してください"];
            [panel setNameFieldStringValue:@"U2FPrivKey"];
            [panel setAllowedFileTypes:@[@"pem"]];
            
        } else if (self.pathType == PATH_CSR) {
            [panel setMessage:@"作成する証明書要求ファイル(CSR)名を指定してください"];
            [panel setNameFieldStringValue:@"U2FCertReq"];
            [panel setAllowedFileTypes:@[@"csr"]];
            
        } else if (self.pathType == PATH_CERT) {
            [panel setMessage:@"作成する自己署名証明書ファイル(CRT)名を指定してください"];
            [panel setNameFieldStringValue:@"U2FSelfCer"];
            [panel setAllowedFileTypes:@[@"crt"]];
        }
        [panel setPrompt:@"作成"];
        
        return panel;
    }

    - (void)panelWillCreatePath:(NSSavePanel *)panel {
        // ファイル保存パネルをモーダル表示
        AppDelegate * __weak weakSelf = self;
        [panel beginSheetModalForWindow:self.window completionHandler:^(NSInteger result) {
            [panel orderOut:self];
            if (result != NSFileHandlingPanelOKButton) {
                return;
            }
            // ファイルパスが作成された時の処理
            [weakSelf panelDidCreatePath:panel];
        }];
    }

    - (void)panelDidCreatePath:(NSSavePanel *)panel {
        // ファイル保存パネルで作成されたファイルパスを取得
        NSString *filePath = [[panel URL] path];
        // 仮コード：ファイル保存パネルで選択されたファイルパスを表示する
        if (self.pathType == PATH_SKEY) {
            [self displaySuccessPopupMessage:filePath];
            
        } else if (self.pathType == PATH_CSR) {
            [self displaySuccessPopupMessage:filePath];
            
        } else if (self.pathType == PATH_CERT) {
            [self displaySuccessPopupMessage:filePath];
        }
    }

#pragma mark - Call back from ToolCommand

    - (void)notifyToolCommandMessage:(NSString *)message {
        // 画面上のテキストエリアにメッセージを表示する
        if (message) {
            [self appendLogMessage:message];
        }
    }

    - (void)toolCommandDidCreateBleRequest {
        // BLEデバイス接続処理に移る
        [self.toolBLECentral centralManagerWillConnect];
    }

    - (void)toolCommandDidFail:(NSString *)errorMessage {
        // エラーメッセージを画面表示
        if (errorMessage) {
            [self appendLogMessage:errorMessage];
        }
        // デバイス接続を切断
        [self.toolBLECentral centralManagerWillDisconnect];
        // 失敗メッセージを表示
        [self displayEndMessage:false];
        [self enableButtons:true];
    }

    - (void)toolCommandDidSuccess {
        // デバイス接続を切断
        [self.toolBLECentral centralManagerWillDisconnect];
        // 成功メッセージを表示
        [self displayEndMessage:true];
        [self enableButtons:true];
    }

    - (void)toolCommandDidReceive:(NSDictionary *)u2fResponseDict {
        // U2F処理実行結果をChromeエクステンションに戻す
        [self.toolBLEHelper bleHelperWillSend:u2fResponseDict];
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

    - (void)centralManagerDidFailConnection:(NSString *)errorMessage {
        // エラーメッセージを画面表示
        if (errorMessage) {
            [self appendLogMessage:errorMessage];
        }
        // 失敗メッセージを表示
        [self displayEndMessage:false];
        [self enableButtons:true];
    }

    - (void)notifyCentralManagerMessage:(NSString *)message {
        // 画面上のテキストエリアにメッセージを表示する
        if (message) {
            [self appendLogMessage:message];
        }
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

    - (void)displayEndMessage:(bool)success {
        // 実行中のコマンドに対応する処理名を取得
        NSString *processName = [self.toolCommand processNameOfCommand];
        if (!processName) {
            return;
        }
        
        // 正常終了時のメッセージを、テキストエリアとメッセージボックスの両方に表示させる
        NSString *str = [NSString stringWithFormat:
                         @"%1$@が%2$@しました。", processName, success? @"成功":@"失敗"];
        [self appendLogMessage:str];
        if (success) {
            [self displaySuccessPopupMessage:str];
        } else {
            [self displayErrorPopupMessage:str];
        }
    }

    - (void)displaySuccessPopupMessage:(NSString *)successMessage {
        if (!successMessage) {
            return;
        }
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setAlertStyle:NSAlertStyleInformational];
        [alert setMessageText:successMessage];
        [alert runModal];
    }

    - (void)displayErrorPopupMessage:(NSString *)errorMessage {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setAlertStyle:NSAlertStyleCritical];
        if (errorMessage) {
            [alert setMessageText:errorMessage];
        } else {
            [alert setMessageText:@"不明なエラーが発生しました。"];
        }
        [alert runModal];
    }

#pragma mark - Call back from ToolBLEHelper

    - (void)bleHelperDidReceive:(NSArray<NSDictionary *> *)bleHelperMessages {
        // Chromeエクステンションからの受信データによりU2F処理を実行
        [self.toolCommand setU2FProcessParameter:COMMAND_U2F_PROCESS
                               bleHelperMessages:bleHelperMessages];
        [self.toolCommand toolCommandWillCreateBleRequest:COMMAND_U2F_PROCESS];
    }

    - (void)toolCommandDidSetup:(bool)result {
        [self enableButtons:true];
        if (!result) {
            // 失敗メッセージを表示
            [self displayEndMessage:false];
            return;
        }
        // 成功メッセージを表示
        [self displayEndMessage:true];
    }

@end
