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

    @property (nonatomic) ToolPreferenceWindow  *toolPreferenceWindow;

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

    - (void)commandWillProcess:(ToolPreferenceCommandType)commandType {
        switch (commandType) {
            case COMMAND_AUTH_PARAM_GET:
                // 仮のコードです。
                [self setServiceUUIDString:@"422E0000-E141-11E5-A837-0800200C9A66"];
                [self setServiceUUIDScanSec:5];
                // 画面に制御を戻す
                [[self toolPreferenceWindow]
                    toolPreferenceCommandDidProcess:commandType success:true message:@""];
                break;
            default:
                break;
        }
    }

@end
