//
//  UtilityCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/06/07.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "ToolLogFile.h"
#import "UtilityCommand.h"
#import "UtilityWindow.h"
#import "ToolVersionWindow.h"

@interface UtilityCommand ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) UtilityWindow                *utilityWindow;
    @property (nonatomic) ToolVersionWindow            *toolVersionWindow;

@end

@implementation UtilityCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 画面のインスタンスを生成
            [self setUtilityWindow:[[UtilityWindow alloc] initWithWindowNibName:@"UtilityWindow"]];
            [self setToolVersionWindow:[[ToolVersionWindow alloc] initWithWindowNibName:@"ToolVersionWindow"]];
            // バージョン情報をセット
            NSString *version = [NSString stringWithFormat:MSG_FORMAT_APP_VERSION,
                                 [[NSBundle mainBundle] infoDictionary][@"CFBundleShortVersionString"]];
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

@end
