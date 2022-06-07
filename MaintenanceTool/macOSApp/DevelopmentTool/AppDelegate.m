//
//  AppDelegate.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2021/10/14.
//
#import "AppCommonMessage.h"
#import "AppDelegate.h"
#import "ToolLogFile.h"

@interface AppDelegate ()

    @property (assign) IBOutlet NSWindow    *window;
    @property (assign) IBOutlet NSButton    *buttonFIDO;
    @property (assign) IBOutlet NSButton    *buttonUtility;
    @property (assign) IBOutlet NSButton    *buttonQuit;
    @property (assign) IBOutlet NSTextView  *textView;
@end

@implementation AppDelegate

    - (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
        // アプリケーション開始ログを出力
        [[ToolLogFile defaultLogger] infoWithFormat:MSG_APP_LAUNCHED, [self getAppVersionString]];

        // テキストエリアの初期化
        [[self textView] setFont:[NSFont fontWithName:@"Courier" size:12]];
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
        // TODO: 仮の実装です。
        [self appendLogMessage:MSG_APP_FUNC_NOT_SUPPORTED];
    }

    - (IBAction)buttonUtilityDidPress:(id)sender {
        // TODO: 仮の実装です。
        [self appendLogMessage:MSG_APP_FUNC_NOT_SUPPORTED];
    }

    - (IBAction)buttonQuitDidPress:(id)sender {
        // このアプリケーションを終了する
        [NSApp terminate:sender];
    }

    - (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
        // ウィンドウをすべて閉じたらアプリケーションを終了
        return YES;
    }

#pragma mark - Private Functions

    - (NSString *)getAppVersionString {
        return [[NSBundle mainBundle] infoDictionary][@"CFBundleShortVersionString"];
    }

@end
