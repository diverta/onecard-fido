//
//  UtilityCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/06.
//
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "ToolCommonFunc.h"
#import "ToolLogFile.h"
#import "ToolVersionWindow.h"
#import "UtilityCommand.h"
#import "UtilityWindow.h"

@interface UtilityCommand () <AppHIDCommandDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) UtilityWindow                *utilityWindow;
    @property (nonatomic) ToolVersionWindow            *toolVersionWindow;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;

@end

@implementation UtilityCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // 画面のインスタンスを生成
            [self setUtilityWindow:[[UtilityWindow alloc] initWithWindowNibName:@"UtilityWindow"]];
            [self setToolVersionWindow:[[ToolVersionWindow alloc] initWithWindowNibName:@"ToolVersionWindow"]];
            // ヘルパークラスのインスタンスを生成
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
            // バージョン情報をセット
            NSString *version = [NSString stringWithFormat:MSG_FORMAT_APP_VERSION, [ToolCommonFunc getAppVersionString]];
            [[self toolVersionWindow] setVersionInfoWithToolName:MSG_APP_NAME toolVersion:version toolCopyright:MSG_APP_COPYRIGHT];
        }
        return self;
    }

    - (void)utilityWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self utilityWindow] setParentWindowRef:parentWindow];
        [[self utilityWindow] setCommandRef:self];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self utilityWindow] window];
        UtilityCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf utilityWindowDidClose:self modalResponse:response];
        }];
    }

    - (bool)checkUSBHIDConnectionOnWindow:(NSWindow *)window {
        // USBポートに接続されていない場合はfalse
        bool connected = [[self appHIDCommand] checkUSBHIDConnection];
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:window connected:connected];
    }

#pragma mark - Perform functions

    - (void)utilityWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self utilityWindow] close];
        // 実行コマンドにより処理分岐
        switch ([[self utilityWindow] commandToPerform]) {
            case COMMAND_VIEW_APP_VERSION:
                // バージョン情報画面を表示
                [self toolVersionWindowWillOpen:self parentWindow:[self parentWindow]];
                break;
            case COMMAND_VIEW_LOG_FILE:
                // ログファイル格納フォルダーを表示
                [self viewLogFileFolder];
                break;
            default:
                // メイン画面に制御を戻す
                break;
        }
    }

    - (void)viewLogFileFolder {
        // ログファイル格納フォルダーをFinderで表示
        NSURL *url = [NSURL fileURLWithPath:[[ToolLogFile defaultLogger] logFilePathString] isDirectory:false];
        NSArray *fileURLs = [NSArray arrayWithObjects:url, nil];
        [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:fileURLs];
    }

#pragma mark - Version window

    - (void)toolVersionWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 画面に親画面参照をセット
        [[self toolVersionWindow] setParentWindowRef:parentWindow];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self toolVersionWindow] window];
        UtilityCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf toolVersionWindowDidClose:self modalResponse:response];
        }];
    }

    - (void)toolVersionWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self toolVersionWindow] close];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時でアプリケーションに制御を戻す
        if (success == false) {
            [self notifyCommandTerminated:[self commandName] message:errorMessage success:success fromWindow:[self parentWindow]];
            return;
        }
        // 実行コマンドにより処理分岐
        switch (command) {
            default:
                // メイン画面に制御を戻す
                [self notifyCommandTerminated:[self commandName] message:nil success:success fromWindow:[self parentWindow]];
                break;
        }
    }

@end
