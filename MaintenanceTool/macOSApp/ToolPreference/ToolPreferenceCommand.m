//
//  ToolPreferenceCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/10/24.
//
#import <Foundation/Foundation.h>
#import "ToolPreferenceCommand.h"
#import "ToolPreferenceWindow.h"

@interface ToolPreferenceCommand () <ToolHIDCommandDelegate>

    @property (nonatomic) TransportType         transportType;
    @property (nonatomic) ToolBLECommand        *toolBLECommand;
    @property (nonatomic) ToolHIDCommand        *toolHIDCommand;
    @property (nonatomic) ToolPreferenceWindow  *toolPreferenceWindow;
    @property (nonatomic) NSData                *hmacSecretSalt;

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
        [self setToolHIDCommand:[[ToolHIDCommand alloc] initWithDelegate:self]];

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

    - (void)toolPreferenceCommandWillProcess:(ToolPreferenceCommandType)commandType {
    }

#pragma mark - Call back from ToolHIDCommand

    - (void)hidCommandDidProcess:(NSString *)processNameOfCommand
                          result:(bool)result message:(NSString *)message {
    }

    - (void)notifyToolCommandMessage:(NSString *)message {
    }

@end
