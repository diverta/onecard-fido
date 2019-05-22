//
//  ToolCTAP2HealthCheckCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/02.
//
#import <Foundation/Foundation.h>

#import "ToolCommon.h"
#import "ToolCommonMessage.h"
#import "ToolCTAP2HealthCheckCommand.h"
#import "PinCodeParamWindow.h"
#import "FIDODefines.h"
#import "CBORDecoder.h"
#import "CBOREncoder.h"
#import "debug_log.h"

@interface ToolCTAP2HealthCheckCommand ()

    @property (nonatomic) ToolHIDCommand     *toolHIDCommand;
    @property (nonatomic) PinCodeParamWindow *pinCodeParamWindow;
    @property (nonatomic) NSData             *hmacSecretSalt;

@end

@implementation ToolCTAP2HealthCheckCommand

    - (id)init {
        self = [super init];
        // 使用するダイアログを生成
        [self setPinCodeParamWindow:[[PinCodeParamWindow alloc]
                                    initWithWindowNibName:@"PinCodeParamWindow"]];
        // テストデータ（salt）を生成
        [self setHmacSecretSalt:[self createHmacSecretSalt]];
        NSLog(@"ToolCTAP2HealthCheckCommand initialized");
        return self;
    }

    - (NSData *)createHmacSecretSalt {
        NSData *salt1 =
        [ToolCommon generateHexBytesFrom:
            @"124dc843bb8ba61f035a7d0938251f5dd4cbfc96f5453b130d890a1cdbae3220"];
        NSData *salt2 =
        [ToolCommon generateHexBytesFrom:
            @"23be84e16cd6ae529049f1f1bbe9ebb3a6db3c870c3e99245e0d1c06b747deb3"];
        NSMutableData *salt = [[NSMutableData alloc] initWithData:salt1];
        [salt appendData:salt2];
        return salt;
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
        // makeCredentialリクエストを生成して戻す
        status_code = ctap2_cbor_encode_make_credential(
                            ctap2_cbor_decode_agreement_pubkey_X(),
                            ctap2_cbor_decode_agreement_pubkey_Y(),
                            ctap2_cbor_decrypted_pin_token());
        if (status_code == CTAP1_ERR_SUCCESS) {
            return [[NSData alloc] initWithBytes:ctap2_cbor_encode_request_bytes()
                                          length:ctap2_cbor_encode_request_bytes_size()];
        } else {
            NSLog(@"CBOREncoder error: %s", log_debug_message());
            return nil;
        }
    }

    - (bool)parseMakeCredentialResponseWith:(NSData *)makeCredentialResponse {
        // MakeCredentialレスポンスからクレデンシャルIDを抽出
        uint8_t *response = (uint8_t *)[makeCredentialResponse bytes];
        size_t   responseSize = [makeCredentialResponse length];
        uint8_t  status_code = ctap2_cbor_decode_make_credential(response, responseSize);
        if (status_code != CTAP1_ERR_SUCCESS) {
            return false;
        }
        // レスポンス内に"hmac-secret"拡張が含まれていたらその旨をログ表示
        if (ctap2_cbor_decode_ext_hmac_secret()->flag) {
            NSLog(@"parseMakeCredentialResponseWith: 'hmac-secret':true");
        }
        // for debug
        // NSLog(@"parseMakeCredentialResponseWith: credential id %@",
        //       [[NSData alloc] initWithBytes:ctap2_cbor_decode_credential_id()
        //                              length:ctap2_cbor_decode_credential_id_size()]);
        return true;
    }

    - (NSData *)generateGetAssertionRequestWith:(NSData *)getPinTokenResponse userPresence:(bool)up {
        // GetPinTokenレスポンスからPINトークンを抽出
        uint8_t *pinTokenResp = (uint8_t *)[getPinTokenResponse bytes];
        size_t   pinTokenRespSize = [getPinTokenResponse length];
        uint8_t  status_code = ctap2_cbor_decode_pin_token(pinTokenResp, pinTokenRespSize);
        if (status_code != CTAP1_ERR_SUCCESS) {
            return nil;
        }
        // getAssertionリクエストを生成して戻す
        status_code = ctap2_cbor_encode_get_assertion(
                            ctap2_cbor_decode_agreement_pubkey_X(),
                            ctap2_cbor_decode_agreement_pubkey_Y(),
                            ctap2_cbor_decrypted_pin_token(),
                            ctap2_cbor_decode_credential_id(),
                            ctap2_cbor_decode_credential_id_size(),
                            (uint8_t *)[[self hmacSecretSalt] bytes], up);
        if (status_code == CTAP1_ERR_SUCCESS) {
            return [[NSData alloc] initWithBytes:ctap2_cbor_encode_request_bytes()
                                          length:ctap2_cbor_encode_request_bytes_size()];
        } else {
            NSLog(@"CBOREncoder error: %s", log_debug_message());
            return nil;
        }
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
