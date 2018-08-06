#import "AppDelegate.h"
#import "ToolBLECentral.h"
#import "ToolHIDHelper.h"
#import "ToolCommand.h"
#import "ToolCommonMessage.h"

@interface AppDelegate ()
    <ToolBLECentralDelegate, ToolHIDHelperDelegate, ToolCommandDelegate>

    @property (assign) IBOutlet NSWindow    *window;
    @property (assign) IBOutlet NSButton    *buttonHide;
    @property (assign) IBOutlet NSButton    *buttonQuit;
    @property (assign) IBOutlet NSTextView  *textView;
    @property (assign) IBOutlet NSTextField *textVersion;

    @property (assign) IBOutlet NSMenu      *menuStatus;
    @property (assign) IBOutlet NSMenuItem  *menuItemOpen;
    @property (assign) IBOutlet NSMenuItem  *menuItemQuit;

    @property (nonatomic) NSStatusItem      *statusItem;

    @property (nonatomic) ToolCommand       *toolCommand;
    @property (nonatomic) ToolBLECentral    *toolBLECentral;
    @property (nonatomic) ToolHIDHelper     *toolHIDHelper;

    @property (nonatomic) NSUInteger         bleConnectionRetryCount;
    @property (nonatomic) bool               bleTransactionStarted;

    @property (nonatomic) NSString          *lastCommandMessage;
    @property (nonatomic) bool               lastCommandSuccess;

@end

@implementation AppDelegate

    - (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
        self.toolBLECentral = [[ToolBLECentral alloc] initWithDelegate:self];
        self.toolHIDHelper  = [[ToolHIDHelper alloc]  initWithDelegate:self];
        self.toolCommand    = [[ToolCommand alloc]    initWithDelegate:self];

        self.textView.font = [NSFont fontWithName:@"Courier" size:12];

        // アプリケーションをステータスバーに表示する
        [self setupStatusItem];
        
        // アプリケーションのバージョンを表示する
        NSString *version = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
        NSString *build = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"CFBundleVersion"];
        NSString *versionString = [NSString stringWithFormat:@"Version %1$@ (%2$@)", version, build];
        [[self textVersion] setStringValue:versionString];
    }

    - (void)setupStatusItem {
        // アプリケーションをステータスバーに表示する
        NSStatusBar *systemStatusBar = [NSStatusBar systemStatusBar];
        [self setStatusItem:
            [systemStatusBar statusItemWithLength:NSVariableStatusItemLength]];
        [[self statusItem] setHighlightMode:YES];
        [[self statusItem] setImage:[NSImage imageNamed:@"StatusBarIconTemplate"]];
        [[self statusItem] setMenu:[self menuStatus]];
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
        [self.buttonHide setEnabled:enabled];
        [self.buttonQuit setEnabled:enabled];
    }


    - (IBAction)buttonQuitDidPress:(id)sender {
        // このアプリケーションを終了する
        [NSApp terminate:sender];
    }

    - (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
        // ウィンドウをすべて閉じたらアプリケーションを終了
        return YES;
    }

    - (IBAction)buttonHideDidPress:(id)sender {
        // 画面を隠す
        [[self window] setAlphaValue:0.0];
    }

    - (IBAction)menuItemOpenDidSelect:(id)sender {
        // 画面を再表示し、他のアプリよりも全面に表示させる
        [[self window] setIsVisible:true];
        [[self window] setAlphaValue:1.0];
        [NSApp activateIgnoringOtherApps:YES];
    }

    - (IBAction)menuItemQuitDidSelect:(id)sender {
        // このアプリケーションを終了する
        [NSApp terminate:sender];
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
        
        if ([self lastCommandSuccess] == false) {
            // リザルト失敗時はU2FHID_ERRORレスポンスをHIDデバイスに転送
            [[self toolHIDHelper] hidHelperWillSendErrorResponse:0x7f];
        } else {
            // U2FレスポンスをHIDデバイスに転送
            [[self toolHIDHelper] hidHelperWillSend:[[self toolCommand] bleResponseData]];
        }
        
        // ボタンを活性化し、メッセージを表示
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

#pragma mark - Call back from ToolHIDHelper

    - (void)hidHelperDidReceive:(NSData *)hidHelperMessages {
        // HIDデバイスから受信したメッセージをToolCommandに引き渡し
        [[self toolCommand] setU2FHIDProcessParameter:COMMAND_U2F_HID_PROCESS
                                     hidHelperMessage:hidHelperMessages];
        // ToolCommandの該当コマンドを実行する
        [self enableButtons:false];
        [[self toolCommand] toolCommandWillCreateBleRequest:COMMAND_U2F_HID_PROCESS];
    }

@end
