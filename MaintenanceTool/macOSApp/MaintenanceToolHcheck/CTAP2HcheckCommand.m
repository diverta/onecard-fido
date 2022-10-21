//
//  CTAP2HcheckCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/13.
//
#import "AppBLECommand.h"
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "AppHIDCommand.h"
#import "CBORDecoder.h"
#import "CBOREncoder.h"
#import "CTAP2HcheckCommand.h"
#import "FIDODefines.h"
#import "HcheckCommand.h"
#import "ToolCommon.h"
#import "ToolLogFile.h"
#import "debug_log.h"

@interface CTAP2HcheckCommand () <AppHIDCommandDelegate, AppBLECommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                  delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppBLECommand            *appBLECommand;
    @property (nonatomic) AppHIDCommand            *appHIDCommand;
    // 実行対象コマンド／サブコマンドを保持
    @property (nonatomic) Command                   command;
    @property (nonatomic) uint8_t                   cborCommand;
    @property (nonatomic) uint8_t                   subCommand;
    // ログインテストカウンター
    @property (nonatomic) uint8_t                   getAssertionCount;
    // HMAC暗号を保持
    @property (nonatomic) NSData                   *hmacSecretSalt;
    // ヘルスチェック処理のパラメーターを保持
    @property (nonatomic) HcheckCommandParameter   *commandParameter;
    // 使用トランスポートを保持
    @property (nonatomic) TransportType             transportType;

@end

@implementation CTAP2HcheckCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 上位クラスの参照を保持
            [self setDelegate:delegate];
            // ヘルパークラスのインスタンスを生成
            [self setAppBLECommand:[[AppBLECommand alloc] initWithDelegate:self]];
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
            // テストデータ（HMAC暗号）を生成
            [self setHmacSecretSalt:[self createHmacSecretSalt]];
        }
        return self;
    }

    - (NSData *)createHmacSecretSalt {
        NSData *salt1 = [ToolCommon generateHexBytesFrom:@"124dc843bb8ba61f035a7d0938251f5dd4cbfc96f5453b130d890a1cdbae3220"];
        NSData *salt2 = [ToolCommon generateHexBytesFrom:@"23be84e16cd6ae529049f1f1bbe9ebb3a6db3c870c3e99245e0d1c06b747deb3"];
        NSMutableData *salt = [[NSMutableData alloc] initWithData:salt1];
        [salt appendData:salt2];
        return salt;
    }

#pragma mark - Command/subcommand process

    - (void)doRequestHidCtap2HealthCheck:(id)parameterRef {
        // パラメーターを保持
        [self setCommandParameter:(HcheckCommandParameter *)parameterRef];
        // トランスポートをUSB HIDに設定
        [self setTransportType:TRANSPORT_HID];
        // CTAPHID_INITから実行
        [self setCommand:COMMAND_TEST_MAKE_CREDENTIAL];
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doResponseHIDCtap2Init {
        // CTAPHID_INIT応答後の処理を実行
        switch ([self command]) {
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                [self doRequestCommandGetKeyAgreement];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self commandDidProcess:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

    - (void)doRequestCommandGetKeyAgreement {
        // 実行対象サブコマンドを退避
        [self setCborCommand:CTAP2_CMD_CLIENT_PIN];
        [self setSubCommand:CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT];
        // メッセージを編集
        NSData *message = [self generateGetKeyAgreementRequest];
        if (message == nil) {
            [self commandDidProcess:false message:nil];
            return;
        }
        // getKeyAgreementサブコマンドを実行
        [self doRequestCtap2CborCommand:COMMAND_CTAP2_GET_KEY_AGREEMENT withData:message];
    }

    - (void)doResponseCommandGetKeyAgreement:(NSData *)message {
        // CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT応答後の処理を実行
        switch ([self command]) {
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                // PINトークン取得処理を続行
                [self doRequestCommandGetPinToken:message];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self commandDidProcess:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

    - (void)doRequestCommandGetPinToken:(NSData *)message {
        // 実行するコマンドを退避
        [self setCborCommand:CTAP2_CMD_CLIENT_PIN];
        [self setSubCommand:CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN];
        // レスポンスされたCBORを抽出
        NSData *cborBytes = [ToolCommon extractCBORBytesFrom:message];
        // メッセージを編集
        NSData *request = [self generateClientPinTokenGetRequestWith:cborBytes];
        if (request == nil) {
            [self commandDidProcess:false message:nil];
            return;
        }
        // getPINTokenサブコマンドを実行
        [self doRequestCtap2CborCommand:COMMAND_CTAP2_GET_PIN_TOKEN withData:request];
    }

    - (void)doResponseCommandGetPinToken:(NSData *)message {
        // レスポンスをチェックし、内容がNGであれば処理終了
        if ([self checkStatusCode:message] == false) {
            [self commandDidProcess:false message:nil];
            return;
        }
        // CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN応答後の処理を実行
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
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self commandDidProcess:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

    - (void)doRequestCommandMakeCredential:(NSData *)message {
        // 実行するコマンドを退避
        [self setCborCommand:CTAP2_CMD_MAKE_CREDENTIAL];
        // レスポンスされたCBORを抽出
        NSData *cborBytes = [ToolCommon extractCBORBytesFrom:message];
        // メッセージを編集
        NSData *request = [self generateMakeCredentialRequestWith:cborBytes];
        if (request == nil) {
            [self commandDidProcess:false message:nil];
            return;
        }
        // authenticatorMakeCredentialコマンドを実行
        [self doRequestCtap2CborCommand:COMMAND_TEST_MAKE_CREDENTIAL withData:request];
    }

    - (void)doResponseCommandMakeCredential:(NSData *)message {
        // レスポンスをチェックし、内容がNGであれば処理終了
        if ([self checkStatusCode:message] == false) {
            [self commandDidProcess:false message:nil];
            return;
        }
        // レスポンスされたCBORを抽出
        NSData *cborBytes = [ToolCommon extractCBORBytesFrom:message];
        // MakeCredentialレスポンスを解析して保持
        if ([self parseMakeCredentialResponseWith:cborBytes] == false) {
            [self commandDidProcess:false message:nil];
            return;
        }
        // CTAP2ヘルスチェックのログインテストを実行
        [self setGetAssertionCount:1];
        [self setCommand:COMMAND_TEST_GET_ASSERTION];
        // BLEの場合は、CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENTから再実行
        if ([self transportType] == TRANSPORT_BLE) {
            [self doRequestCommandGetKeyAgreement];
        }
        // HIDの場合は、CTAPHID_INITから再実行
        if ([self transportType] == TRANSPORT_HID) {
            [[self appHIDCommand] doRequestCtapHidInit];
        }
    }

    - (void)doRequestCommandGetAssertion:(NSData *)message {
        // 実行するコマンドを退避
        [self setCborCommand:CTAP2_CMD_GET_ASSERTION];
        // レスポンスされたCBORを抽出
        NSData *cborBytes = [ToolCommon extractCBORBytesFrom:message];
        // メッセージを編集し、GetAssertionコマンドを実行
        // ２回目のコマンド実行では、基板上のボタン押下によるユーザー所在確認が必要
        bool testUserPresenceNeeded = ([self getAssertionCount] == 2);
        NSData *request = [self generateGetAssertionRequestWith:cborBytes userPresence:testUserPresenceNeeded];
        if (request == nil) {
            [self commandDidProcess:false message:nil];
            return;
        }
        if (testUserPresenceNeeded) {
            // リクエスト転送の前に、基板上のボタンを押してもらうように促すメッセージを画面表示
            [self displayMessage:MSG_HCHK_CTAP2_LOGIN_TEST_START];
            [self displayMessage:MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT1];
            [self displayMessage:MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT2];
            [self displayMessage:MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT3];
        }
        // authenticatorGetAssertionコマンドを実行
        [self doRequestCtap2CborCommand:COMMAND_TEST_GET_ASSERTION withData:request];
    }

    - (void)doResponseCommandGetAssertion:(NSData *)message {
        // レスポンスをチェックし、内容がNGであれば処理終了
        if ([self checkStatusCode:message] == false) {
            [self commandDidProcess:false message:nil];
            return;
        }
        // レスポンスされたCBORを抽出
        NSData *cborBytes = [ToolCommon extractCBORBytesFrom:message];
        // GetAssertionレスポンスを解析
        // ２回目のコマンド実行では、認証器から受領したHMAC暗号の内容検証が必要
        bool verifySaltNeeded = ([self getAssertionCount] == 2);
        if ([self parseGetAssertionResponseWith:cborBytes verifySalt:verifySaltNeeded] == false) {
            [self commandDidProcess:false message:nil];
            return;
        }
        // ２回目のテストが成功したら上位クラスに制御を戻して終了
        if (verifySaltNeeded) {
            [self commandDidProcess:true message:nil];
            return;
        }
        // CTAP2ヘルスチェックのログインテストを再度実行
        [self setGetAssertionCount:[self getAssertionCount] + 1];
        [self setCommand:COMMAND_TEST_GET_ASSERTION];
        // BLEの場合は、CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENTから再実行
        if ([self transportType] == TRANSPORT_BLE) {
            [self doRequestCommandGetKeyAgreement];
        }
        // HIDの場合は、CTAPHID_INITから再実行
        if ([self transportType] == TRANSPORT_HID) {
            [[self appHIDCommand] doRequestCtapHidInit];
        }
    }

    - (void)doResponseCtap2HealthCheck:(bool)result message:(NSString *)message {
        // 上位クラスに制御を戻す
        [[self delegate] doResponseCtap2HealthCheck:result message:message];
    }

#pragma mark - BLE Command/subcommand process

    - (void)doRequestBleCtap2HealthCheck:(id)parameterRef {
        // パラメーターを保持
        [self setCommandParameter:(HcheckCommandParameter *)parameterRef];
        // トランスポートをBLEに設定
        [self setTransportType:TRANSPORT_BLE];
        // CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENTから実行
        [self setCommand:COMMAND_TEST_MAKE_CREDENTIAL];
        [self doRequestCommandGetKeyAgreement];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時で上位クラスに制御を戻す
        if (success == false) {
            [self doResponseCtap2HealthCheck:false message:errorMessage];
            return;
        }
        // 実行コマンドにより処理分岐
        switch (command) {
            case COMMAND_HID_CTAP2_INIT:
                [self doResponseHIDCtap2Init];
                break;
            case COMMAND_CTAP2_GET_KEY_AGREEMENT:
                [self doResponseCommandGetKeyAgreement:response];
                break;
            case COMMAND_CTAP2_GET_PIN_TOKEN:
                [self doResponseCommandGetPinToken:response];
                break;
            case COMMAND_TEST_MAKE_CREDENTIAL:
                [self doResponseCommandMakeCredential:response];
                break;
            case COMMAND_TEST_GET_ASSERTION:
                [self doResponseCommandGetAssertion:response];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self doResponseCtap2HealthCheck:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

#pragma mark - Call back from AppBLECommand

    - (void)didResponseCommand:(Command)command response:(NSData *)response {
        // 実行コマンドにより処理分岐
        switch (command) {
            case COMMAND_CTAP2_GET_KEY_AGREEMENT:
                [self doResponseCommandGetKeyAgreement:response];
                break;
            case COMMAND_CTAP2_GET_PIN_TOKEN:
                [self doResponseCommandGetPinToken:response];
                break;
            case COMMAND_TEST_MAKE_CREDENTIAL:
                [self doResponseCommandMakeCredential:response];
                break;
            case COMMAND_TEST_GET_ASSERTION:
                [self doResponseCommandGetAssertion:response];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、一旦ヘルパークラスに制御を戻す
                [[self appBLECommand] commandDidProcess:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

    - (void)didCompleteCommand:(Command)command success:(bool)success errorMessage:(NSString *)errorMessage {
        // 上位クラスに制御を戻す
        [self doResponseCtap2HealthCheck:success message:errorMessage];
    }

#pragma mark - Private functions

    - (void)doRequestCtap2CborCommand:(Command)command withData:(NSData *)data {
        // コマンドリクエストを、BLE／HIDトランスポート経由で実行
        if ([self transportType] == TRANSPORT_BLE) {
            [[self appBLECommand] doRequestCommand:command withCMD:HID_CMD_MSG withData:data];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self appHIDCommand] doRequestCtap2Command:command withCMD:HID_CMD_CTAPHID_CBOR withData:data];
        }
    }

    - (void)commandDidProcess:(bool)success message:(NSString *)message {
        // コマンド実行完了後の処理
        if ([self transportType] == TRANSPORT_BLE) {
            // 一旦ヘルパークラスに制御を戻し、BLE切断処理を実行
            [[self appBLECommand] commandDidProcess:success message:message];
        }
        if ([self transportType] == TRANSPORT_HID) {
            // 上位クラスに制御を戻す
            [self doResponseCtap2HealthCheck:success message:message];
        }
    }

    - (NSData *)generateGetKeyAgreementRequest {
        // GetKeyAgreementリクエストを生成して戻す
        uint8_t status_code = ctap2_cbor_encode_get_agreement_key();
        if (status_code == CTAP1_ERR_SUCCESS) {
            return [[NSData alloc] initWithBytes:ctap2_cbor_encode_request_bytes() length:ctap2_cbor_encode_request_bytes_size()];
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
            [[ToolLogFile defaultLogger] errorWithFormat:@"ctap2_cbor_decode_get_agreement_key error: %d", status_code];
            [[ToolLogFile defaultLogger] hexdump:keyAgreementResponse];
            return nil;
        }
        // getPinTokenリクエストを生成して戻す
        char *pin_cur = (char *)[[[self commandParameter] pin] UTF8String];
        status_code = ctap2_cbor_encode_client_pin_token_get(
                            ctap2_cbor_decode_agreement_pubkey_X(),
                            ctap2_cbor_decode_agreement_pubkey_Y(),
                            pin_cur);
        if (status_code == CTAP1_ERR_SUCCESS) {
            return [[NSData alloc] initWithBytes:ctap2_cbor_encode_request_bytes()
                                          length:ctap2_cbor_encode_request_bytes_size()];
        } else {
            [[ToolLogFile defaultLogger] errorWithFormat:@"CBOREncoder error: %s", log_debug_message()];
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
            [[ToolLogFile defaultLogger] errorWithFormat:@"CBOREncoder error: %s", log_debug_message()];
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
            [[ToolLogFile defaultLogger]
             debug:@"authenticatorMakeCredential: HMAC Secret Extension available"];
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
            [[ToolLogFile defaultLogger] errorWithFormat:@"CBOREncoder error: %s", log_debug_message()];
            return nil;
        }
    }

    - (bool)parseGetAssertionResponseWith:(NSData *)getAssertionResponse verifySalt:(bool)verifySalt {
        // GetAssertionレスポンスを解析
        uint8_t *response = (uint8_t *)[getAssertionResponse bytes];
        size_t   responseSize = [getAssertionResponse length];
        uint8_t  status_code = ctap2_cbor_decode_get_assertion(response, responseSize, verifySalt);
        if (status_code != CTAP1_ERR_SUCCESS) {
            [[ToolLogFile defaultLogger]
             errorWithFormat:@"parseGetAssertionResponseWith failed(0x%02x)", status_code];
            return false;
        }
        // レスポンス内に"hmac-secret"拡張が含まれていない場合はここで終了
        if (ctap2_cbor_decode_ext_hmac_secret()->output_size == 0) {
            return true;
        }
        // ２回目のGetAssertion時は、認証器から受領したHMAC暗号の内容検証を行う
        if (verifySalt) {
            bool success = ctap2_cbor_decode_verify_salt();
            [[ToolLogFile defaultLogger]
             debugWithFormat:@"authenticatorGetAssertion: HMAC Secret verification %@",
             success ? @"success" : @"failed"];
            return success;
        } else {
            return true;
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

    - (void)displayMessage:(NSString *)message {
        // メッセージを画面表示
        [[self delegate] notifyMessage:message];
    }

@end
