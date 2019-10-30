//
//  ToolPreferenceCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/10/24.
//
#import <Foundation/Foundation.h>
#import "ToolPreferenceCommand.h"
#import "ToolPreferenceWindow.h"

@interface ToolPreferenceCommand ()

    @property (nonatomic, weak) AppDelegate         *delegate;
    @property (nonatomic) ToolPreferenceWindow      *toolPreferenceWindow;
    @property (nonatomic) ToolPreferenceCommandType commandType;

    // コマンドを実行するためのリクエストデータを保持
    @property (nonatomic) NSData *processData;

@end

@implementation ToolPreferenceCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
        }

        // 使用するダイアログを生成
        [self setToolPreferenceWindow:[[ToolPreferenceWindow alloc]
                                     initWithWindowNibName:@"ToolPreferenceWindow"]];
        return self;
    }

#pragma mark - Main process

    - (void)generateRequestCommandAuthParamGet {
        // 仮のコードです。
        NSString *csv = @"1,234,5";
        NSData *data = [csv dataUsingEncoding:NSUTF8StringEncoding];
        [self setProcessData:data];
    }

    - (void)parseResponseCommandAuthParamGet {
        // 仮のコードです。
        [self setServiceUUIDString:@"422E0000-E141-11E5-A837-0800200C9A66"];
        [self setServiceUUIDScanSec:@"5"];
    }

#pragma mark - Interface for AppDelegate

    - (void)toolPreferenceWillProcess:(ToolPreferenceCommandType)commandType {
        [self setCommandType:commandType];
        switch ([self commandType]) {
            case COMMAND_AUTH_PARAM_GET:
                [self generateRequestCommandAuthParamGet];
                break;
            default:
                break;
        }

        // AppDelegate経由でコマンドを実行
        [[self delegate] toolPreferenceWillProcess:COMMAND_TOOL_PREF_PARAM withData:[self processData]];
    }

    - (void)toolPreferenceDidProcess:(Command)command
                                 CMD:(uint8_t)cmd response:(NSData *)resp
                              result:(bool)result message:(NSString *)message {
        NSLog(@"toolPreferenceDidProcess: cmd[%02x] response[%@]", cmd, resp);
        switch ([self commandType]) {
            case COMMAND_AUTH_PARAM_GET:
                [self parseResponseCommandAuthParamGet];
                break;
            default:
                break;
        }
        // 画面に制御を戻す
        [[self toolPreferenceWindow] toolPreferenceCommandDidProcess:[self commandType]
                                                             success:result message:message];
    }

#pragma mark - Interface for ToolPreferenceWindow

    - (void)toolPreferenceWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ダイアログの親ウィンドウを保持
        [[self toolPreferenceWindow] setParentWindow:parentWindow];
        [[self toolPreferenceWindow] setToolPreferenceCommand:self];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self toolPreferenceWindow] window];
        ToolPreferenceCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf toolPreferenceWindowDidClose:sender modalResponse:response];
        }];
    }

    - (void)toolPreferenceWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self toolPreferenceWindow] close];
        // AppDelegateに制御を戻す（ポップアップメッセージは表示しない）
        AppDelegate *app = (AppDelegate *)[self delegate];
        [app toolPreferenceWindowDidClose];
    }

@end
