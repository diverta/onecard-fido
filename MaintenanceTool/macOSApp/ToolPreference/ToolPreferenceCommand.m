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

    @property (nonatomic) TransportType         transportType;
    @property (nonatomic) ToolBLECommand        *toolBLECommand;
    @property (nonatomic) ToolHIDCommand        *toolHIDCommand;
    @property (nonatomic) ToolPreferenceWindow  *toolPreferenceWindow;
    @property (nonatomic) NSData                *hmacSecretSalt;

@end

@implementation ToolPreferenceCommand

    - (id)init {
        self = [super init];
        // 使用するダイアログを生成
        [self setToolPreferenceWindow:[[ToolPreferenceWindow alloc]
                                     initWithWindowNibName:@"ToolPreferenceWindow"]];
        return self;
    }

    - (void)setTransportParam:(TransportType)type
               toolBLECommand:(ToolBLECommand *)ble toolHIDCommand:(ToolHIDCommand *)hid {
        [self setTransportType:type];
        [self setToolBLECommand:ble];
        [self setToolHIDCommand:hid];
    }

#pragma mark - Communication with dialog

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
        // AppDelegateに制御を戻す
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] toolPreferenceWindowDidClose];
        }
    }

@end
