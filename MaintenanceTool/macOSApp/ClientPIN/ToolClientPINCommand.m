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
#import "CBORDecoder.h"
#import "CBOREncoder.h"
#import "debug_log.h"

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
        return self;
    }

#pragma mark - Command functions

    - (NSData *)generateClientPinSetRequestWith:(NSData *)keyAgreementResponse {
        // GetKeyAgreementレスポンスから公開鍵を抽出
        uint8_t *keyAgreement = (uint8_t *)[keyAgreementResponse bytes];
        size_t   keyAgreementSize = [keyAgreementResponse length];
        uint8_t  status_code = ctap2_cbor_decode_get_agreement_key(keyAgreement, keyAgreementSize);
        if (status_code != CTAP1_ERR_SUCCESS) {
            return nil;
        }

        // for debug
        // NSLog(@"pubkey_X %@", [[NSData alloc] initWithBytes:ctap2_cbor_decode_agreement_pubkey_X() length:32]);
        // NSLog(@"pubkey_Y %@", [[NSData alloc] initWithBytes:ctap2_cbor_decode_agreement_pubkey_Y() length:32]);

        // SetPINまたはChangePINリクエストを生成して戻す
        char *pin_new = (char *)[[self pinNew] UTF8String];
        char *pin_old = NULL;
        if ([[self pinOld] length] != 0) {
            pin_old = (char *)[[self pinOld] UTF8String];
        }
        status_code = ctap2_cbor_encode_client_pin_set_or_change(
                        ctap2_cbor_decode_agreement_pubkey_X(), ctap2_cbor_decode_agreement_pubkey_Y(),
                        pin_new, pin_old);
        if (status_code == CTAP1_ERR_SUCCESS) {
            return [[NSData alloc] initWithBytes:ctap2_cbor_encode_request_bytes()
                                          length:ctap2_cbor_encode_request_bytes_size()];
        } else {
            NSLog(@"CBOREncoder error: %s", log_debug_message());
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
        [[self toolHIDCommand] hidHelperWillProcess:[self pinCommand]];
    }

@end
