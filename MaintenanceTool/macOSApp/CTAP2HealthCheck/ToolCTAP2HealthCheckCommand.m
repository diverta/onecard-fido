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

    // ログインテストカウンター
    @property (nonatomic) uint8_t   getAssertionCount;

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

    - (void)doCTAP2Request:(Command)command {
        // 実行対象コマンドを退避
        [self setCommand:command];
        switch ([self command]) {
            case COMMAND_CLIENT_PIN_SET:
            case COMMAND_CLIENT_PIN_CHANGE:
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                [self doRequestCommandGetKeyAgreement];
                break;
            case COMMAND_AUTH_RESET:
                [self doRequestCommandAuthReset];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、画面に制御を戻す
                [self doResponseToAppDelegate:true message:nil];
                break;
        }
    }

    - (void)doCTAP2Response:(Command)command responseMessage:(NSData *)message {
        // レスポンスをチェックし、内容がNGであれば処理終了
        if ([self checkStatusCode:message] == false) {
            [self doResponseToAppDelegate:false message:nil];
            return;
        }
        
        switch ([self cborCommand]) {
            case CTAP2_CMD_CLIENT_PIN:
                [self doResponseCommandClientPin:message];
                break;
            case CTAP2_CMD_MAKE_CREDENTIAL:
                [self doResponseCommandMakeCredential:message];
                break;
            case CTAP2_CMD_GET_ASSERTION:
                [self doResponseCommandGetAssertion:message];
                break;
            case CTAP2_CMD_RESET:
                // 画面に制御を戻す
                [self doResponseToAppDelegate:true message:nil];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、画面に制御を戻す
                [self doResponseToAppDelegate:false message:nil];
                break;
        }
    }

    - (void)doResponseCommandClientPin:(NSData *)message {
        // レスポンスされたCBORを抽出
        NSData *cborBytes = [self extractCBORBytesFrom:message];
        
        switch ([self subCommand]) {
            case CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT:
                [self doResponseCommandGetKeyAgreement:cborBytes];
                break;
            case CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN:
                [self doResponseCommandGetPinToken:cborBytes];
                break;
            case CTAP2_SUBCMD_CLIENT_PIN_SET:
            case CTAP2_SUBCMD_CLIENT_PIN_CHANGE:
                // 画面に制御を戻す
                [self doResponseToAppDelegate:true message:nil];
                break;
            default:
                // 画面に制御を戻す
                [self doResponseToAppDelegate:false message:nil];
                break;
        }
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

    - (void)doRequestCommandGetKeyAgreement {
        // 実行対象サブコマンドを退避
        [self setCborCommand:CTAP2_CMD_CLIENT_PIN];
        [self setSubCommand:CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT];
        // メッセージを編集
        NSData *message = [self generateGetKeyAgreementRequest];
        if (message == nil) {
            [self doResponseToAppDelegate:false message:nil];
            return;
        }
        // getKeyAgreementサブコマンドを実行
        if ([self transportType] == TRANSPORT_BLE) {
            [[self toolBLECommand] doBLECommandRequestFrom:message cmd:BLE_CMD_MSG];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] doRequest:message CID:[self CID] CMD:HID_CMD_CTAPHID_CBOR];
        }
    }

    - (void)doResponseCommandGetKeyAgreement:(NSData *)message {
        switch ([self command]) {
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                // PINトークン取得処理を続行
                [self doRequestCommandGetPinToken:message];
                break;
            case COMMAND_CLIENT_PIN_SET:
                // PIN新規設定処理を続行
                [self setSubCommand:CTAP2_SUBCMD_CLIENT_PIN_SET];
                [[self toolHIDCommand] doClientPinSetOrChange:message CID:[self CID]];
                break;
            case COMMAND_CLIENT_PIN_CHANGE:
                // PIN変更処理を続行
                [self setSubCommand:CTAP2_SUBCMD_CLIENT_PIN_CHANGE];
                [[self toolHIDCommand] doClientPinSetOrChange:message CID:[self CID]];
                break;
            default:
                // 画面に制御を戻す
                [self doResponseToAppDelegate:true message:nil];
                break;
        }
    }

    - (void)doRequestCommandGetPinToken:(NSData *)message {
        // 実行するコマンドを退避
        [self setCborCommand:CTAP2_CMD_CLIENT_PIN];
        [self setSubCommand:CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN];
        // メッセージを編集
        NSData *request = [self generateClientPinTokenGetRequestWith:message];
        if (request == nil) {
            [self doResponseToAppDelegate:false message:nil];
            return;
        }
        // getPINTokenサブコマンドを実行
        if ([self transportType] == TRANSPORT_BLE) {
            [[self toolBLECommand] doBLECommandRequestFrom:request cmd:BLE_CMD_MSG];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] doRequest:request CID:[self CID] CMD:HID_CMD_CTAPHID_CBOR];
        }
    }

    - (void)doResponseCommandGetPinToken:(NSData *)message {
        switch ([self command]) {
            case COMMAND_TEST_MAKE_CREDENTIAL:
                // ユーザー登録テスト処理を続行
                [self doRequestCommandMakeCredential:message];
                break;
            case COMMAND_TEST_GET_ASSERTION:
                // ログインテスト処理を続行
                [self doRequestCommandGetAssertion:message];
                break;
            default:
                // 画面に制御を戻す
                [self doResponseToAppDelegate:true message:nil];
                break;
        }
    }

    - (void)doRequestCommandMakeCredential:(NSData *)message {
        // 実行するコマンドを退避
        [self setCborCommand:CTAP2_CMD_MAKE_CREDENTIAL];
        // メッセージを編集
        NSData *request = [self generateMakeCredentialRequestWith:message];
        if (request == nil) {
            [self doResponseToAppDelegate:false message:nil];
            return;
        }
        // authenticatorMakeCredentialコマンドを実行
        if ([self transportType] == TRANSPORT_BLE) {
            [[self toolBLECommand] doBLECommandRequestFrom:request cmd:BLE_CMD_MSG];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] doRequest:request CID:[self CID] CMD:HID_CMD_CTAPHID_CBOR];
        }
    }

    - (void)doResponseCommandMakeCredential:(NSData *)message {
        // レスポンスされたCBORを抽出
        NSData *cborBytes = [self extractCBORBytesFrom:message];
        // MakeCredentialレスポンスを解析して保持
        if ([self parseMakeCredentialResponseWith:cborBytes] == false) {
            [self doResponseToAppDelegate:false message:nil];
            return;
        }
        // CTAP2ヘルスチェックのログインテストを実行
        [self setGetAssertionCount:1];
        if ([self transportType] == TRANSPORT_BLE) {
            [self doCTAP2Request:COMMAND_TEST_GET_ASSERTION];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TEST_GET_ASSERTION];
        }
    }

    - (void)doRequestCommandGetAssertion:(NSData *)message {
        // 実行するコマンドを退避
        [self setCborCommand:CTAP2_CMD_GET_ASSERTION];
        // メッセージを編集し、GetAssertionコマンドを実行
        // ２回目のコマンド実行では、MAIN SW押下によるユーザー所在確認が必要
        bool testUserPresenceNeeded = ([self getAssertionCount] == 2);
        NSData *request = [self generateGetAssertionRequestWith:message
                                                   userPresence:testUserPresenceNeeded];
        if (request == nil) {
            [self doResponseToAppDelegate:false message:nil];
            return;
        }
        if (testUserPresenceNeeded) {
            // リクエスト転送の前に、基板上のMAIN SWを押してもらうように促すメッセージを画面表示
            [self displayMessage:MSG_HCHK_CTAP2_LOGIN_TEST_START];
            [self displayMessage:MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT1];
            [self displayMessage:MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT2];
            [self displayMessage:MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT3];
        }
        // authenticatorGetAssertionコマンドを実行
        if ([self transportType] == TRANSPORT_BLE) {
            [[self toolBLECommand] doBLECommandRequestFrom:request cmd:BLE_CMD_MSG];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] doRequest:request CID:[self CID] CMD:HID_CMD_CTAPHID_CBOR];
        }
    }

    - (void)doResponseCommandGetAssertion:(NSData *)message {
        // レスポンスされたCBORを抽出
        NSData *cborBytes = [self extractCBORBytesFrom:message];
        // GetAssertionレスポンスを解析
        // ２回目のコマンド実行では、認証器から受領したsaltの内容検証が必要
        bool verifySaltNeeded = ([self getAssertionCount] == 2);
        if ([self parseGetAssertionResponseWith:cborBytes
                                     verifySalt:verifySaltNeeded] == false) {
            [self doResponseToAppDelegate:false message:nil];
            return;
        }
        // ２回目のテストが成功したら画面に制御を戻して終了
        if (verifySaltNeeded) {
            [self doResponseToAppDelegate:true message:nil];
            return;
        }
        // CTAP2ヘルスチェックのログインテストを再度実行
        [self setGetAssertionCount:[self getAssertionCount] + 1];
        if ([self transportType] == TRANSPORT_BLE) {
            [self doCTAP2Request:COMMAND_TEST_GET_ASSERTION];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TEST_GET_ASSERTION];
        }
    }

    - (void)doRequestCommandAuthReset {
        // 実行するサブコマンドを退避
        [self setCborCommand:CTAP2_CMD_RESET];
        // リクエスト転送の前に、基板上のMAIN SWを押してもらうように促すメッセージを表示
        [self displayMessage:MSG_CLEAR_PIN_CODE_COMMENT1];
        [self displayMessage:MSG_CLEAR_PIN_CODE_COMMENT2];
        [self displayMessage:MSG_CLEAR_PIN_CODE_COMMENT3];
        // メッセージを編集し、authenticatorResetコマンドを実行
        char commandByte[] = {CTAP2_CMD_RESET};
        NSData *message = [[NSData alloc] initWithBytes:commandByte length:sizeof(commandByte)];
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] doRequest:message CID:[self CID] CMD:HID_CMD_CTAPHID_CBOR];
        }
    }

#pragma mark - Common methods

    - (void)displayMessage:(NSString *)message {
        // メッセージを画面表示
        if ([self transportType] == TRANSPORT_BLE) {
            [[self toolBLECommand] displayMessage:message];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self toolHIDCommand] displayMessage:message];
        }
    }

    - (void)doResponseToAppDelegate:(bool)result message:(NSString *)message {
        if ([self transportType] == TRANSPORT_BLE) {
            // BLE接続を切断してから、アプリケーションに制御を戻す
            [[self toolBLECommand] commandDidProcess:result message:message];
        }
        if ([self transportType] == TRANSPORT_HID) {
            // 即時でアプリケーションに制御を戻す
            [[[self toolHIDCommand] delegate] hidCommandDidProcess:[self command] result:result message:message];
        }
    }

    - (NSData *)extractCBORBytesFrom:(NSData *)responseMessage {
        // CBORバイト配列（レスポンスの２バイト目以降）を抽出
        size_t cborLength = [responseMessage length] - 1;
        NSData *cborBytes = [responseMessage subdataWithRange:NSMakeRange(1, cborLength)];
        return cborBytes;
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
