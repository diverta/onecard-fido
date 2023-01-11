//
//  VendorFunctionCommand.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2023/01/11.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "ToolCommonFunc.h"
#import "ToolLogFile.h"
#import "VendorFunctionCommand.h"
#import "VendorFunctionWindow.h"
#import "ToolVersionWindow.h"

@interface VendorFunctionCommand ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) VendorFunctionWindow         *vendorFunctionWindow;
    @property (nonatomic) ToolVersionWindow            *toolVersionWindow;

@end

@implementation VendorFunctionCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 画面のインスタンスを生成
            [self setVendorFunctionWindow:[[VendorFunctionWindow alloc] initWithWindowNibName:@"VendorFunctionWindow"]];
            [self setToolVersionWindow:[[ToolVersionWindow alloc] initWithWindowNibName:@"ToolVersionWindow"]];
            // バージョン情報をセット
            NSString *version = [NSString stringWithFormat:MSG_FORMAT_APP_VERSION, [ToolCommonFunc getAppVersionString]];
            [[self toolVersionWindow] setVersionInfoWithToolName:MSG_APP_NAME toolVersion:version toolCopyright:MSG_APP_COPYRIGHT];
        }
        return self;
    }

    - (void)vendorFunctionWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self vendorFunctionWindow] setParentWindowRef:parentWindow];
        [[self vendorFunctionWindow] setCommandRef:self];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self vendorFunctionWindow] window];
        VendorFunctionCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf vendorFunctionWindowDidClose:self modalResponse:response];
        }];
    }

#pragma mark - Perform functions

    - (void)vendorFunctionWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self vendorFunctionWindow] close];
        // 実行コマンドにより処理分岐
        switch ([[self vendorFunctionWindow] commandToPerform]) {
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
        VendorFunctionCommand * __weak weakSelf = self;
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
