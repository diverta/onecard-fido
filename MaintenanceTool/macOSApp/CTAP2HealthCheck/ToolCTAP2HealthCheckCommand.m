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

    @property (nonatomic) TransportType       transportType;
    @property (nonatomic) ToolBLECommand     *toolBLECommand;
    @property (nonatomic) ToolHIDCommand     *toolHIDCommand;
    @property (nonatomic) PinCodeParamWindow *pinCodeParamWindow;
    @property (nonatomic) NSData             *hmacSecretSalt;

    // 実行対象コマンド／サブコマンドを保持
    @property (nonatomic) Command   command;
    @property (nonatomic) uint8_t   cborCommand;
    @property (nonatomic) uint8_t   subCommand;

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

    - (void)setTransportParam:(TransportType)type
               toolBLECommand:(ToolBLECommand *)ble toolHIDCommand:(ToolHIDCommand *)hid {
        [self setTransportType:type];
        [self setToolBLECommand:ble];
        [self setToolHIDCommand:hid];
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

    - (NSData *)generateGetKeyAgreementRequest {
        // GetKeyAgreementリクエストを生成して戻す
        uint8_t status_code = ctap2_cbor_encode_get_agreement_key();
        if (status_code == CTAP1_ERR_SUCCESS) {
            return [[NSData alloc] initWithBytes:ctap2_cbor_encode_request_bytes()
                                          length:ctap2_cbor_encode_request_bytes_size()];
        } else {
            return nil;
        }
    }

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

    - (bool)parseGetAssertionResponseWith:(NSData *)getAssertionResponse verifySalt:(bool)verifySalt {
        // GetAssertionレスポンスを解析
        uint8_t *response = (uint8_t *)[getAssertionResponse bytes];
        size_t   responseSize = [getAssertionResponse length];
        uint8_t  status_code = ctap2_cbor_decode_get_assertion(response, responseSize, verifySalt);
        if (status_code != CTAP1_ERR_SUCCESS) {
            NSLog(@"parseGetAssertionResponseWith failed(0x%02x)", status_code);
            return false;
        }
        // レスポンス内に"hmac-secret"拡張が含まれていない場合はここで終了
        if (ctap2_cbor_decode_ext_hmac_secret()->output_size == 0) {
            return true;
        }
        // ２回目のGetAssertion時は、認証器から受領したsaltの内容検証を行う
        if (verifySalt) {
            bool success = ctap2_cbor_decode_verify_salt();
            NSLog(@"parseGetAssertionResponseWith: hmac-secret-salt verify %@",
                  success ? @"success" : @"failed");
            return success;
        } else {
            return true;
        }
    }

#pragma mark - Command/subcommand process

    - (bool)doCTAP2Response:(Command)command responseMessage:(NSData *)response {
        // レスポンスをチェックし、内容がNGであれば処理終了
        if ([self checkStatusCode:response] == false) {
            return false;
        }
        
        return true;
    }

    - (bool)checkStatusCode:(NSData *)responseMessage {
        // レスポンスデータが揃っていない場合はNG
        if (responseMessage == nil || [responseMessage length] == 0) {
            [self displayMessage:MSG_OCCUR_UNKNOWN_ERROR];
            return false;
        }
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *requestBytes = (uint8_t *)[responseMessage bytes];
        switch (requestBytes[0]) {
            case CTAP1_ERR_SUCCESS:
                return true;
            case CTAP2_ERR_PIN_INVALID:
            case CTAP2_ERR_PIN_AUTH_INVALID:
                [self displayMessage:MSG_CTAP2_ERR_PIN_INVALID];
                break;
            case CTAP2_ERR_PIN_BLOCKED:
                [self displayMessage:MSG_CTAP2_ERR_PIN_BLOCKED];
                break;
            case CTAP2_ERR_PIN_AUTH_BLOCKED:
                [self displayMessage:MSG_CTAP2_ERR_PIN_AUTH_BLOCKED];
                break;
            case CTAP2_ERR_PIN_NOT_SET:
                [self displayMessage:MSG_CTAP2_ERR_PIN_NOT_SET];
                break;
            case CTAP2_ERR_VENDOR_KEY_CRT_NOT_EXIST:
                [self displayMessage:MSG_OCCUR_SKEYNOEXIST_ERROR];
                break;
            default:
                [self displayMessage:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
        return false;
    }

    - (void)displayMessage:(NSString *)message {
        // メッセージを画面表示
        if ([self transportType] == TRANSPORT_BLE) {
            [[self toolBLECommand] displayMessage:message];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] displayMessage:message];
        }
    }

    - (bool)doGetKeyAgreementCommandRequest:(Command)command {
        // 実行対象サブコマンドを退避
        [self setCommand:command];
        [self setCborCommand:CTAP2_CMD_CLIENT_PIN];
        [self setSubCommand:CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT];
        // メッセージを編集
        NSData *message = [self generateGetKeyAgreementRequest];
        if (message == nil) {
            return false;
        }
        // GetKeyAgreementサブコマンドを実行
        if ([self transportType] == TRANSPORT_BLE) {
            [[self toolBLECommand] doBLECommandRequestFrom:message cmd:0x83];
        }
        return true;
    }

#pragma mark - Communication with dialog

    - (void)pinCodeParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ダイアログの親ウィンドウを保持
        [[self pinCodeParamWindow] setParentWindow:parentWindow];
        [[self pinCodeParamWindow] setToolCTAP2HealthCheckCommand:self];
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
            [self pinCodeParamWindowDidCancel];
            return;
        }
        // CTAP2ヘルスチェックを実行
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TEST_MAKE_CREDENTIAL];
        }
        if ([self transportType] == TRANSPORT_BLE) {
            [[self toolBLECommand] bleCommandWillProcess:COMMAND_TEST_MAKE_CREDENTIAL];
        }
    }

    - (void)pinCodeParamWindowDidCancel {
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] pinCodeParamWindowDidClose];
        }
        if ([self transportType] == TRANSPORT_BLE) {
            [[self toolBLECommand] pinCodeParamWindowDidClose];
        }
    }

@end
