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
#import "FIDODefines.h"
#import "CBOREncoder.h"

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

    - (NSData *)generateGetKeyAgreementRequest:(Command)command {
        // GetKeyAgreementリクエストを生成して戻す
        uint8_t status_code = ctap2_cbor_encode_get_agreement_key();
        if (status_code == CTAP1_ERR_SUCCESS) {
            return [[NSData alloc] initWithBytes:ctap2_cbor_encode_request_bytes()
                                          length:ctap2_cbor_encode_request_bytes_size()];
        } else {
            return nil;
        }
    }

#pragma mark - Communication with dialog

    - (void)setPinParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow
                          toolCommand:(ToolHIDCommand *)toolCommand {
        // ダイアログの親ウィンドウを保持
        [[self setPinParamWindow] setParentWindow:parentWindow];
        [[self setPinParamWindow] setToolClientPINCommand:self];
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
        // 画面を閉じる
        [[self setPinParamWindow] close];
        // キャンセルボタンがクリックされた場合は、そのままAppDelegateに制御を戻す
        if (modalResponse == NSModalResponseCancel) {
            [[self toolHIDCommand] setPinParamWindowDidClose];
            return;
        }
        // PINコード新規設定／変更を実行
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_CLIENT_PIN];
    }

@end
