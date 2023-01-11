//
//  AppDelegate.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/14.
//
#import "AppCommand.h"
#import "AppCommonMessage.h"
#import "AppDelegate.h"
#import "BLESettingCommand.h"
#import "DFUCommand.h"
#import "FIDOSettingCommand.h"
#import "HcheckCommand.h"
#import "ToolCommonFunc.h"
#import "ToolPGPCommand.h"
#import "ToolPIVCommand.h"
#import "ToolPopupWindow.h"
#import "ToolLogFile.h"
#import "UtilityCommand.h"

@interface AppDelegate () <AppCommandDelegate>

    @property (assign) IBOutlet NSWindow    *window;
    @property (assign) IBOutlet NSButton    *buttonBLESetting;
    @property (assign) IBOutlet NSButton    *buttonFIDOSetting;
    @property (assign) IBOutlet NSButton    *buttonOATHSetting;
    @property (assign) IBOutlet NSButton    *buttonSetPivParam;
    @property (assign) IBOutlet NSButton    *buttonDFU;
    @property (assign) IBOutlet NSButton    *buttonSetPgpParam;
    @property (assign) IBOutlet NSButton    *buttonHealthCheck;
    @property (assign) IBOutlet NSButton    *buttonUtility;
    @property (assign) IBOutlet NSButton    *buttonQuit;
    @property (assign) IBOutlet NSTextView  *textView;
    @property (assign) IBOutlet NSMenuItem  *menuItemVendor;

    // クラスの参照を保持
    @property (nonatomic) BLESettingCommand     *bleSettingCommand;
    @property (nonatomic) DFUCommand            *dfuCommand;
    @property (nonatomic) FIDOSettingCommand    *fidoSettingCommand;
    @property (nonatomic) HcheckCommand         *hcheckCommand;
    @property (nonatomic) UtilityCommand        *utilityCommand;
    @property (nonatomic) ToolPIVCommand        *toolPIVCommand;
    @property (nonatomic) ToolPGPCommand        *toolPGPCommand;

@end

@implementation AppDelegate

    - (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
        // アプリケーション開始ログを出力
        [[ToolLogFile defaultLogger] infoWithFormat:MSG_APP_LAUNCHED, [ToolCommonFunc getAppVersionString]];

        // コマンドクラスの初期化
        [self setBleSettingCommand:[[BLESettingCommand alloc] initWithDelegate:self]];
        [self setDfuCommand:[[DFUCommand alloc] initWithDelegate:self]];
        [self setFidoSettingCommand:[[FIDOSettingCommand alloc] initWithDelegate:self]];
        [self setHcheckCommand:[[HcheckCommand alloc] initWithDelegate:self]];
        [self setUtilityCommand:[[UtilityCommand alloc] initWithDelegate:self]];
        [self setToolPIVCommand:[[ToolPIVCommand alloc] initWithDelegate:self]];
        [self setToolPGPCommand:[[ToolPGPCommand alloc] initWithDelegate:self]];

        // テキストエリアの初期化
        [[self textView] setFont:[NSFont fontWithName:@"Courier" size:12]];
        
        // ベンダー向け機能を有効化
        [[self menuItemVendor] setHidden:([ToolCommonFunc isVendorMaintenanceTool] == false)];
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
        [[self buttonBLESetting] setEnabled:enabled];
        [[self buttonFIDOSetting] setEnabled:enabled];
        [[self buttonSetPivParam] setEnabled:enabled];
        [[self buttonDFU] setEnabled:enabled];
        [[self buttonSetPgpParam] setEnabled:enabled];
        [[self buttonOATHSetting] setEnabled:enabled];
        [[self buttonHealthCheck] setEnabled:enabled];
        [[self buttonUtility] setEnabled:enabled];
        [[self buttonQuit] setEnabled:enabled];
        [[self menuItemVendor] setEnabled:enabled];
    }

    - (IBAction)buttonBLESettingDidPress:(id)sender {
        // BLE設定画面を開く
        [[self bleSettingCommand] bleSettingWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)buttonFIDOSettingDidPress:(id)sender {
        // FIDO設定画面を開く
        [[self fidoSettingCommand] fidoSettingWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)buttonOATHSettingDidPress:(id)sender {
        [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil
                                       withObject:self forSelector:@selector(displayCommandResultDone)
                                     parentWindow:[self window]];
    }

    - (IBAction)buttonSetPivParamDidPress:(id)sender {
        // PIV機能設定画面を表示
        [[self toolPIVCommand] commandWillOpenPreferenceWindowWithParent:[self window]];
    }

    - (IBAction)buttonDFUDidPress:(id)sender {
        // ファームウェア更新画面を表示
        [[self dfuCommand] DFUWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)buttonSetPgpParamDidPress:(id)sender {
        // OpenPGP機能設定画面を表示
        [[self toolPGPCommand] commandWillOpenPreferenceWindowWithParent:[self window]];
    }

    - (IBAction)buttonQuitDidPress:(id)sender {
        // このアプリケーションを終了する
        [NSApp terminate:sender];
    }

    - (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
        // ウィンドウをすべて閉じたらアプリケーションを終了
        return YES;
    }

    - (IBAction)buttonHealthCheckDidPress:(id)sender {
        // ヘルスチェック実行画面を開く
        [[self hcheckCommand] hcheckWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)buttonUtilityDidPress:(id)sender {
        // ユーティリティー画面を開く
        [[self utilityCommand] utilityWindowWillOpen:self parentWindow:[self window]];
    }

    - (IBAction)menuItemVendorDidSelect:(id)sender {
        // TODO: ベンダー向け機能画面を開く
        [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil
                                       withObject:self forSelector:@selector(displayCommandResultDone)
                                     parentWindow:[self window]];
    }

#pragma mark - Common method called by callback

    - (void)notifyAppCommandMessage:(NSString *)message {
        // 画面上のテキストエリアにメッセージを表示する
        if (message) {
            [self appendLogMessage:message];
        }
    }

    - (void)commandStartedProcess:(NSString *)processNameOfCommand {
        if (processNameOfCommand) {
            // コマンド開始メッセージを画面表示し、ログファイルにも出力
            NSString *startMsg = [NSString stringWithFormat:MSG_FORMAT_START_MESSAGE,
                                  processNameOfCommand];
            [self notifyAppCommandMessage:startMsg];
            [[ToolLogFile defaultLogger] info:startMsg];
        }
    }

    - (void)commandDidProcess:(bool)result message:(NSString *)message processNameOfCommand:(NSString *)processNameOfCommand {
        // 処理失敗時は、引数に格納されたエラーメッセージを画面出力
        if (result == false) {
            [self notifyAppCommandMessage:message];
        }
        // コマンド名称を取得
        if (processNameOfCommand == nil) {
            // ボタンを活性化
            [self enableButtons:true];
            return;
        }
        // テキストエリアとポップアップの両方に表示させる処理終了メッセージを作成
        NSString *str = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE,
                         processNameOfCommand, result? MSG_SUCCESS:MSG_FAILURE];
        // メッセージを画面のテキストエリアに表示
        [self notifyAppCommandMessage:str];
        // メッセージをログファイルに出力してから、ポップアップを表示
        [self displayCommandResult:result withMessage:str];
    }

    - (void)displayCommandResult:(bool)result withMessage:(NSString *)str{
        // メッセージをログファイルに出力してから、ポップアップを表示-->ボタンを活性化
        if (result) {
            [[ToolLogFile defaultLogger] info:str];
            [[ToolPopupWindow defaultWindow] informational:str informativeText:nil withObject:self forSelector:@selector(displayCommandResultDone)
                                              parentWindow:[self window]];
        } else {
            [[ToolLogFile defaultLogger] error:str];
            [[ToolPopupWindow defaultWindow] critical:str informativeText:nil withObject:self forSelector:@selector(displayCommandResultDone)
                                         parentWindow:[self window]];
        }
    }

    - (void)displayCommandResultDone {
        // ボタンを活性化
        [self enableButtons:true];
    }

    - (void)enableButtonsOfMainUI:(bool)enable {
        // ボタンを活性化
        [self enableButtons:enable];
    }

    - (void)notifyMessageToMainUI:(NSString *)message {
        // メッセージを画面のテキストエリアに表示
        [self notifyAppCommandMessage:message];
    }

@end
