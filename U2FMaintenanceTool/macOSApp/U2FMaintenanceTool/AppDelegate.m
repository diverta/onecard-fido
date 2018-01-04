#import "AppDelegate.h"
#import "ToolBLECentral.h"
#import "ToolCommand.h"

typedef enum : NSInteger {
    PATH_SKEY = 1,
    PATH_CERT
} PathType;

@interface AppDelegate () <ToolBLECentralDelegate>

    @property (nonatomic) ToolCommand    *toolCommand;
    @property (nonatomic) ToolBLECentral *central;

    @property (nonatomic) PathType  pathType;
    @property (nonatomic) NSString *messageWhenSuccess;
    @property (nonatomic) bool      communicateAsChromeNative;

@end

@implementation AppDelegate

    - (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
        self.toolCommand = [ToolCommand alloc];

        self.central = [[ToolBLECentral alloc] initWithDelegate:self];
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
        [self.central disconnect];
    }

    - (void)appendLogMessage:(NSString *)message {
        self.textView.string = [self.textView.string stringByAppendingFormat:@"%@\n", message];
        [self.textView performSelector:@selector(scrollPageDown:) withObject:nil afterDelay:0];
    }

#pragma mark - Functions for communication with chrome extension

    - (void)setCommunicationMode {
        [self setCommunicateAsChromeNative:false];

        // プログラム起動引数をチェック
        NSArray *commandLineArgs = [[NSProcessInfo processInfo] arguments];
        if ([commandLineArgs count] < 2) {
            return;
        }

        // ２番目の引数がChromeエクステンションIDになっている場合
        // Chromeエクステンションから実行されたと判定し、
        // ネイティブアプリとして動作するようにする
        NSString *extensionID = [commandLineArgs objectAtIndex:1];
        if ([extensionID compare:@"chrome-extension://mmafjllbfijjcejkmnaoioihhfnelodd/"]
            != NSOrderedSame) {
            return;
        }

        // ネイティブアプリ動作に固有の処理を実行
        [self setCommunicateAsChromeNative:true];
        [self setStandardInputNotification];
    }

    - (bool)doBLEHelperCommandWith:(NSData *)receivedData {
        NSString *_receivedString = [[NSString alloc]
                                     initWithData:receivedData encoding:NSUTF8StringEncoding];
        NSLog(@"Received String:[%@]", _receivedString);
 
        // ヘルスチェック実行（仮コードです）
        [self enableButtons:false];
        [self.toolCommand setCommand:COMMAND_TEST_REGISTER];
        [self.central doCommand:self.toolCommand];
        [self setMessageWhenSuccess:@"ヘルスチェックが成功しました。"];

        return true;
    }

    - (void)standardInputDidAvailable:(NSFileHandle *)input {
        // これ以上メッセージを受信できなくなった場合は終了
        NSData *_data = [input availableData];
        if ([_data length] == 0) {
            NSLog(@"End of message from chrome extension");
            return;
        }

        // 最初の４バイトを除去し、受信メッセージテキストを抽出（仮コードです）
        NSLog(@"Received Data:[%@]", _data);
        NSData *_dataString = [_data subdataWithRange:NSMakeRange(4, [_data length] - 4)];

        // BLEと通信を行う
        [self doBLEHelperCommandWith:_dataString];
        
        // 標準出力にエコーバック（仮コードです）
        [[NSFileHandle fileHandleWithStandardOutput] writeData:_data];

        // 次のメッセージが受信できるよう設定
        [input waitForDataInBackgroundAndNotify];
    }

    - (void)setStandardInputNotification {
        // 標準入力をメッセージとして受信できるよう設定
        NSFileHandle *_input = [NSFileHandle fileHandleWithStandardInput];
        [_input waitForDataInBackgroundAndNotify];

        // 標準入力を受信した時の処理
        NSNotificationCenter *_defaultCenter = [NSNotificationCenter defaultCenter];
        [_defaultCenter addObserverForName:NSFileHandleDataAvailableNotification
            object:_input queue:nil
            usingBlock:^(NSNotification *notification){
                [self standardInputDidAvailable:_input];
         }];
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
        [self.toolCommand setCommand:COMMAND_ERASE_BOND];
        [self.central doCommand:self.toolCommand];
        [self setMessageWhenSuccess:@"ペアリング情報削除処理が成功しました。"];
    }

    - (IBAction)button2DidPress:(id)sender {
        // 鍵・証明書削除
        [self enableButtons:false];
        [self.toolCommand setCommand:COMMAND_ERASE_SKEY_CERT];
        [self.central doCommand:self.toolCommand];
        [self setMessageWhenSuccess:@"鍵・証明書削除処理が成功しました。"];
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
        [self.toolCommand setCommand:COMMAND_INSTALL_SKEY];
        [self.toolCommand setSkeyFilePath:self.fieldPath1.stringValue];
        [self.toolCommand setCertFilePath:self.fieldPath2.stringValue];
        [self.central doCommand:self.toolCommand];
        [self setMessageWhenSuccess:@"鍵・証明書インストール処理が成功しました。"];
    }

    - (IBAction)button4DidPress:(id)sender {
        // ヘルスチェック実行
        [self enableButtons:false];
        [self.toolCommand setCommand:COMMAND_TEST_REGISTER];
        [self.central doCommand:self.toolCommand];
        [self setMessageWhenSuccess:@"ヘルスチェックが成功しました。"];
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

#pragma mark - Call back from ToolBLECentral

    - (void)notifyCentralManagerStateUpdate:(CBCentralManagerState)state {
        NSLog(@"centralManagerDidUpdateState: %ld", state);

        if (state == CBCentralManagerStatePoweredOn) {
            // BLEデバイスが有効化された後に、
            // ネイティブアプリ動作モード時に固有な初期化処理を実行する
            [self setCommunicationMode];
        }
    }

    - (void)notifyFailWithMessage:(NSString *)errorMessage {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setAlertStyle:NSAlertStyleCritical];
        if (errorMessage) {
            [alert setMessageText:errorMessage];
        } else {
            [alert setMessageText:@"不明なエラーが発生しました。"];
        }
        [alert runModal];
        [self enableButtons:true];
    }

    - (void)notifySuccess {
        // 正常終了時のメッセージを、テキストエリアとメッセージボックスの両方に表示させる
        if ([self messageWhenSuccess]) {
            [self appendLogMessage:[self messageWhenSuccess]];

            NSAlert *alert = [[NSAlert alloc] init];
            [alert setAlertStyle:NSAlertStyleInformational];
            [alert setMessageText:[self messageWhenSuccess]];
            [alert runModal];
        }
        [self enableButtons:true];
    }

    - (void)notifyMessage:(NSString *)message {
        if (message) {
            [self appendLogMessage:message];
        }
    }

@end
