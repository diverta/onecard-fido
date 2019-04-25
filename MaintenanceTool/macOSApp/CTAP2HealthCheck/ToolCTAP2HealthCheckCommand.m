//
//  ToolCTAP2HealthCheckCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/02.
//
#import <Foundation/Foundation.h>

#import "ToolCommonMessage.h"
#import "ToolCTAP2HealthCheckCommand.h"
#import "PinCodeParamWindow.h"
#import "FIDODefines.h"
#import "CBORDecoder.h"
#import "CBOREncoder.h"
#import "debug_log.h"

@interface ToolCTAP2HealthCheckCommand ()

    @property (nonatomic) ToolHIDCommand    *toolHIDCommand;
    @property (nonatomic) PinCodeParamWindow *pinCodeParamWindow;

@end

@implementation ToolCTAP2HealthCheckCommand

    - (id)init {
        self = [super init];
        // 使用するダイアログを生成
        [self setPinCodeParamWindow:[[PinCodeParamWindow alloc]
                                    initWithWindowNibName:@"PinCodeParamWindow"]];
        NSLog(@"ToolCTAP2HealthCheckCommand initialized");
        return self;
    }

#pragma mark - Command functions

    - (NSData *)generateClientPinTokenGetRequestWith:(NSData *)keyAgreementResponse {
        // GetKeyAgreementレスポンスから公開鍵を抽出
        uint8_t *keyAgreement = (uint8_t *)[keyAgreementResponse bytes];
        size_t   keyAgreementSize = [keyAgreementResponse length];
        uint8_t  status_code = ctap2_cbor_decode_get_agreement_key(keyAgreement, keyAgreementSize);
        if (status_code != CTAP1_ERR_SUCCESS) {
            return nil;
        }
        // getPinTokenリクエストを生成して戻す
        char *pin_cur = (char *)[[self pinCur] UTF8String];
        status_code = ctap2_cbor_encode_client_pin_token_get(
                            ctap2_cbor_decode_agreement_pubkey_X(),
                            ctap2_cbor_decode_agreement_pubkey_Y(),
                            pin_cur);
        if (status_code == CTAP1_ERR_SUCCESS) {
            return [[NSData alloc] initWithBytes:ctap2_cbor_encode_request_bytes()
                                          length:ctap2_cbor_encode_request_bytes_size()];
        } else {
            NSLog(@"CBOREncoder error: %s", log_debug_message());
            return nil;
        }
    }

    - (NSData *)generateMakeCredentialRequestWith:(NSData *)getPinTokenResponse {
        // GetPinTokenレスポンスからPINトークンを抽出
        uint8_t *pinTokenResp = (uint8_t *)[getPinTokenResponse bytes];
        size_t   pinTokenRespSize = [getPinTokenResponse length];
        uint8_t  status_code = ctap2_cbor_decode_pin_token(pinTokenResp, pinTokenRespSize);
        if (status_code != CTAP1_ERR_SUCCESS) {
            return nil;
        }
        
        // for debug
        NSLog(@"decrypted pinToken %@",
              [[NSData alloc] initWithBytes:ctap2_cbor_decrypted_pin_token() length:16]);

        // 仮の実装
        return nil;
    }

#pragma mark - Communication with dialog

    - (void)pinCodeParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow
                           toolCommand:(ToolHIDCommand *)toolCommand {
        // ダイアログの親ウィンドウを保持
        [[self pinCodeParamWindow] setParentWindow:parentWindow];
        [[self pinCodeParamWindow] setToolCTAP2HealthCheckCommand:self];
        [self setToolHIDCommand:toolCommand];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self pinCodeParamWindow] window];
        ToolCTAP2HealthCheckCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf pinCodeParamWindowDidClose:sender modalResponse:response];
        }];
    }

    - (void)pinCodeParamWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self pinCodeParamWindow] close];
        // キャンセルボタンがクリックされた場合は、そのままAppDelegateに制御を戻す
        if (modalResponse == NSModalResponseCancel) {
            [[self toolHIDCommand] pinCodeParamWindowDidClose];
            return;
        }
        // CTAP2ヘルスチェックを実行
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TEST_MAKE_CREDENTIAL];
    }

@end
