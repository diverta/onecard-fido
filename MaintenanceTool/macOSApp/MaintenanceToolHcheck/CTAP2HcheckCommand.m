//
//  CTAP2HcheckCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/13.
//
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

@interface CTAP2HcheckCommand () <AppHIDCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                  delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand            *appHIDCommand;
    // 実行対象コマンド／サブコマンドを保持
    @property (nonatomic) Command                   command;
    @property (nonatomic) uint8_t                   cborCommand;
    @property (nonatomic) uint8_t                   subCommand;
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
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
        }
        return self;
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

    - (void)doRequestCommandGetKeyAgreement {
        // 実行対象サブコマンドを退避
        [self setCborCommand:CTAP2_CMD_CLIENT_PIN];
        [self setSubCommand:CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT];
        // メッセージを編集
        NSData *message = [self generateGetKeyAgreementRequest];
        if (message == nil) {
            [self doResponseCtap2HealthCheck:false message:nil];
            return;
        }
        // getKeyAgreementサブコマンドを実行
        // TODO: BLEトランスポートは後日実装
        if ([self transportType] == TRANSPORT_HID) {
            [[self appHIDCommand] doRequestCtap2Command:COMMAND_CTAP2_GET_KEY_AGREEMENT withCMD:HID_CMD_CTAPHID_CBOR withData:message];
        }
    }

    - (void)doRequestCommandGetPinToken:(NSData *)message {
        // 実行するコマンドを退避
        [self setCborCommand:CTAP2_CMD_CLIENT_PIN];
        [self setSubCommand:CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN];
        // レスポンスされたCBORを抽出
        NSData *cborBytes = [self extractCBORBytesFrom:message];
        // メッセージを編集
        NSData *request = [self generateClientPinTokenGetRequestWith:cborBytes];
        if (request == nil) {
            [self doResponseCtap2HealthCheck:false message:nil];
            return;
        }
        // getPINTokenサブコマンドを実行
        // TODO: BLEトランスポートは後日実装
        if ([self transportType] == TRANSPORT_HID) {
            [[self appHIDCommand] doRequestCtap2Command:COMMAND_CTAP2_GET_PIN_TOKEN withCMD:HID_CMD_CTAPHID_CBOR withData:request];
        }
    }

    - (void)doRequestCommandMakeCredential:(NSData *)message {
        // TODO: 仮の実装です。
        [self doResponseCtap2HealthCheck:true message:nil];
    }

#pragma mark - Common methods

    - (void)doResponseHIDCtap2Init {
        // CTAPHID_INIT応答後の処理を実行
        switch ([self command]) {
            case COMMAND_TEST_MAKE_CREDENTIAL:
                [self doRequestCommandGetKeyAgreement];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self doResponseCtap2HealthCheck:true message:nil];
                break;
        }
    }

    - (void)doResponseCommandGetKeyAgreement:(NSData *)message {
        // CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT応答後の処理を実行
        switch ([self command]) {
            case COMMAND_TEST_MAKE_CREDENTIAL:
                // PINトークン取得処理を続行
                [self doRequestCommandGetPinToken:message];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self doResponseCtap2HealthCheck:true message:nil];
                break;
        }
    }

    - (void)doResponseCommandGetPinToken:(NSData *)message {
        // レスポンスをチェックし、内容がNGであれば処理終了
        if ([self checkStatusCode:message] == false) {
            [self doResponseCtap2HealthCheck:false message:nil];
            return;
        }
        // CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN応答後の処理を実行
        switch ([self command]) {
            case COMMAND_TEST_MAKE_CREDENTIAL:
                // ユーザー登録テスト処理を続行
                [self doRequestCommandMakeCredential:message];
                break;
            default:
                // 画面に制御を戻す
                [self doResponseCtap2HealthCheck:true message:nil];
                break;
        }
    }

    - (void)doResponseCtap2HealthCheck:(bool)result message:(NSString *)message {
        // 上位クラスに制御を戻す
        [[self delegate] doResponseCtap2HealthCheck:result message:message];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
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
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self doResponseCtap2HealthCheck:false message:nil];
                break;
        }
    }

#pragma mark - Private functions

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

    - (NSData *)extractCBORBytesFrom:(NSData *)responseMessage {
        // CBORバイト配列（レスポンスの２バイト目以降）を抽出
        size_t cborLength = [responseMessage length] - 1;
        NSData *cborBytes = [responseMessage subdataWithRange:NSMakeRange(1, cborLength)];
        return cborBytes;
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
