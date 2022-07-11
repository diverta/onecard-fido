//
//  AppDelegate.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2021/10/14.
//
#import "AppCommonMessage.h"
#import "AppDelegate.h"
#import "AppHIDCommand.h"
#import "FIDOSettingCommand.h"
#import "ToolCommonFunc.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"
#import "UtilityCommand.h"

@interface AppDelegate () <AppCommandDelegate, AppHIDCommandDelegate>

    @property (assign) IBOutlet NSWindow        *window;
    @property (assign) IBOutlet NSButton        *buttonFIDO;
    @property (assign) IBOutlet NSButton        *buttonUtility;
    @property (assign) IBOutlet NSButton        *buttonQuit;
    @property (assign) IBOutlet NSTextView      *textView;

    // クラスの参照を保持
    @property (nonatomic) FIDOSettingCommand    *fidoSettingCommand;
    @property (nonatomic) UtilityCommand        *utilityCommand;
    @property (nonatomic) AppHIDCommand         *appHIDCommand;

@end

@implementation AppDelegate

    - (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
        // アプリケーション開始ログを出力
        [[ToolLogFile defaultLogger] infoWithFormat:MSG_APP_LAUNCHED, [ToolCommonFunc getAppVersionString]];

        // テキストエリアの初期化
        [[self textView] setFont:[NSFont fontWithName:@"Courier" size:12]];
        
        // コマンドクラスの初期化
        [self setFidoSettingCommand:[[FIDOSettingCommand alloc] initWithDelegate:self]];
        [self setUtilityCommand:[[UtilityCommand alloc] initWithDelegate:self]];
        [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
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
        [[self buttonFIDO] setEnabled:enabled];
        [[self buttonUtility] setEnabled:enabled];
        [[self buttonQuit] setEnabled:enabled];
    }

    - (IBAction)buttonFIDODidPress:(id)sender {
        // FIDO設定画面を開く
        [[self fidoSettingCommand] FIDOSettingWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)buttonUtilityDidPress:(id)sender {
        // ユーティリティー画面を開く
        [[self utilityCommand] utilityWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)buttonQuitDidPress:(id)sender {
        // このアプリケーションを終了する
        [NSApp terminate:sender];
    }

    - (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
        // ウィンドウをすべて閉じたらアプリケーションを終了
        return YES;
    }

#pragma mark - Call back from AppCommand

    - (void)enableButtonsOfMainUI:(bool)enable {
        // メニュー、ボタンを活性化／非活性化
        [self enableButtons:enable];
    }

    - (void)notifyMessageToMainUI:(NSString *)message {
        // 画面上のテキストエリアにメッセージを表示する
        [self appendLogMessage:message];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
        // HID接続を通知
        [self appendLogMessage:MSG_HID_CONNECTED];
        [[ToolLogFile defaultLogger] info:MSG_HID_CONNECTED];
    }

    - (void)didDetectRemoval {
        // HID接続の切断を通知
        [self appendLogMessage:MSG_HID_REMOVED];
        [[ToolLogFile defaultLogger] info:MSG_HID_REMOVED];
    }

    - (void)didResponseCommand:(Command)command response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
    }

@end
