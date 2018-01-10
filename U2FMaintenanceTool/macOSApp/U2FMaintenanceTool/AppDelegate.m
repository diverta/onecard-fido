#import "AppDelegate.h"
#import "ToolBLECentral.h"
#import "ToolBLEHelper.h"
#import "ToolCommand.h"

typedef enum : NSInteger {
    PATH_SKEY = 1,
    PATH_CERT
} PathType;

@interface AppDelegate ()
    <ToolBLECentralDelegate, ToolBLEHelperDelegate, ToolCommandDelegate>

    @property (nonatomic) ToolCommand    *toolCommand;
    @property (nonatomic) ToolBLECentral *central;
    @property (nonatomic) ToolBLEHelper  *toolBLEHelper;

    @property (nonatomic) PathType  pathType;
    @property (nonatomic) bool      communicateAsChromeNative;

@end

@implementation AppDelegate

    - (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
        self.central       = [[ToolBLECentral alloc] initWithDelegate:self];
        self.toolBLEHelper = [[ToolBLEHelper alloc]  initWithDelegate:self];
        self.toolCommand   = [[ToolCommand alloc]    initWithDelegate:self];

        self.central.serviceUUIDs = @[
            [CBUUID UUIDWithString:@"0000FFFD-0000-1000-8000-00805F9B34FB"]
        ];
        self.central.characteristicUUIDs = @[
            [CBUUID UUIDWithString:@"F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB"],
            [CBUUID UUIDWithString:@"F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB"]
        ];
        self.textView.font = [NSFont fontWithName:@"Courier" size:12];
    }

    - (void)applicationWillTerminate:(NSNotification *)notification {
        [self.central centralManagerWillDisconnect];
    }

    - (void)appendLogMessage:(NSString *)message {
        self.textView.string = [self.textView.string stringByAppendingFormat:@"%@\n", message];
        [self.textView performSelector:@selector(scrollPageDown:) withObject:nil afterDelay:0];
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
        [self.toolCommand setKeyFilePath:COMMAND_INSTALL_SKEY
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
        self.pathType = PATH_SKEY;
        [self buttonPathDidPress:sender];
    }

    - (IBAction)buttonPath2DidPress:(id)sender {
        self.pathType = PATH_CERT;
        [self buttonPathDidPress:sender];
    }

    - (void)buttonPathDidPress:(id)sender {
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

        // ファイル選択パネルをモーダル表示
        AppDelegate * __weak weakSelf = self;
        [panel beginSheetModalForWindow:self.window completionHandler:^(NSInteger result) {
            [panel orderOut:self];
            if (result != NSFileHandlingPanelOKButton) {
                return;
            }
            // 選択されたファイルパスを、テキストフィールドに設定
            NSURL *url = [[panel URLs] objectAtIndex:0];
            [weakSelf setPathField:[url path]];
        }];
    }

    - (void)setPathField:(NSString *)filePath {
        // ファイル選択パネルで選択されたファイルパスを表示する
        if (self.pathType == PATH_SKEY) {
            [self.fieldPath1 setStringValue:filePath];
            [self.fieldPath1 becomeFirstResponder];
        } else {
            [self.fieldPath2 setStringValue:filePath];
            [self.fieldPath2 becomeFirstResponder];
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
        [self.central centralManagerWillConnect];
    }

    - (void)toolCommandDidFail:(NSString *)errorMessage {
        // エラーメッセージを画面表示
        if (errorMessage) {
            [self appendLogMessage:errorMessage];
        }
        // デバイス接続を切断
        [self.central centralManagerWillDisconnect];
        // 失敗メッセージを表示
        [self displayEndMessage:false];
        [self enableButtons:true];
    }

    - (void)toolCommandDidSuccess {
        // デバイス接続を切断
        [self.central centralManagerWillDisconnect];
        // 成功メッセージを表示
        [self displayEndMessage:true];
        [self enableButtons:true];
    }

#pragma mark - Call back from ToolBLECentral

    - (void)notifyCentralManagerStateUpdate:(CBCentralManagerState)state {
        NSLog(@"centralManagerDidUpdateState: %ld", state);

        if (state == CBCentralManagerStatePoweredOn) {
            // BLEデバイスが有効化された後に、
            // ネイティブアプリ動作モード時に固有な初期化処理を実行する
            [self.toolBLEHelper bleHelperWillSetStdinNotification];
        }
    }

    - (void)centralManagerDidConnect {
        // U2F Control Pointに実行コマンドを書込
        [self.central centralManagerWillSend:[self.toolCommand bleRequestArray]];
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
            [self.central centralManagerWillStartResponseTimeout];
        } else {
            // 後続レスポンスがなければ、レスポンスを次処理に引き渡す
            [self.toolCommand toolCommandWillProcessBleResponse];
        }
    }

    - (void)displayEndMessage:(bool)success {
        // 正常終了時のメッセージを、テキストエリアとメッセージボックスの両方に表示させる
        NSString *processName;
        switch ([self.toolCommand command]) {
            case COMMAND_ERASE_BOND:
                processName = @"ペアリング情報削除処理";
                break;
            case COMMAND_ERASE_SKEY_CERT:
                processName = @"鍵・証明書削除処理";
                break;
            case COMMAND_INSTALL_SKEY:
            case COMMAND_INSTALL_CERT:
                processName = @"鍵・証明書インストール処理";
                break;
            case COMMAND_TEST_REGISTER:
            case COMMAND_TEST_AUTH_CHECK:
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                processName = @"ヘルスチェック";
                break;
            default:
                processName = nil;
                break;
        }
        if (processName) {
            NSString *str = [NSString stringWithFormat:@"%1$@が%2$@しました。",
                             processName, success? @"成功":@"失敗"];
            [self appendLogMessage:str];
            if (success) {
                [self displaySuccessPopupMessage:str];
            } else {
                [self displayErrorPopupMessage:str];
            }
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
        NSLog(@"bleHelperMessageDidReceive: %@", bleHelperMessages);
        
        // (仮コード)受信したJSONデータをエコーバック
        [self.toolBLEHelper bleHelperWillSend:[bleHelperMessages objectAtIndex:0]];
    }

@end
