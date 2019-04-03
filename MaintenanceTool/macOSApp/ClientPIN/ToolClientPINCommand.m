//
//  ToolClientPINCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/02.
//
#import <Foundation/Foundation.h>

#import "ToolCommonMessage.h"
#import "ToolHIDCommand.h"
#import "ToolClientPINCommand.h"
#import "SetPinParamWindow.h"

@interface ToolClientPINCommand ()

    @property (nonatomic) ToolHIDCommand    *toolHIDCommand;
    @property (nonatomic) SetPinParamWindow *setPinParamWindow;

@end

@implementation ToolClientPINCommand

    - (id)init {
        self = [super init];
        // 使用するダイアログを生成
        [self setSetPinParamWindow:[[SetPinParamWindow alloc]
                                    initWithWindowNibName:@"SetPinParamWindow"]];
        NSLog(@"ToolClientPINCommand initialized");
        return self;
    }

#pragma mark - Command functions

    - (NSData *)generateSetPinMessage:(Command)command {
        NSLog(@"Set PIN sample start");
        return [[NSData alloc] init];
    }

    - (void)setPinParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow
                          toolCommand:(ToolHIDCommand *)toolCommand {
        // ダイアログの親ウィンドウを保持
        [[self setPinParamWindow] setParentWindow:parentWindow];
        [self setToolHIDCommand:toolCommand];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self setPinParamWindow] window];
        ToolClientPINCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf setPinParamWindowDidClose:sender modalResponse:response];
        }];
    }

    - (void)setPinParamWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じ、AppDelegateに制御を戻す
        [[self setPinParamWindow] close];
        [[self toolHIDCommand] setPinParamWindowDidClose];
    }

@end
