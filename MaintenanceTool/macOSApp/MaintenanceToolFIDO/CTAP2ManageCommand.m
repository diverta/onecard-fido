//
//  CTAP2ManageCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/19.
//
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "CBORDecoder.h"
#import "CBOREncoder.h"
#import "CTAP2ManageCommand.h"
#import "FIDODefines.h"
#import "FIDOSettingCommand.h"
#import "ToolCommon.h"
#import "ToolLogFile.h"
#import "debug_log.h"
#import "fido_client_pin.h"
#import "fido_crypto.h"
#import "tool_ecdh.h"

@interface CTAP2ManageCommand () <AppHIDCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                      delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;
    // 実行対象コマンドを保持
    @property (nonatomic) Command                       command;
    // PINコード設定処理のパラメーターを保持
    @property (nonatomic) FIDOSettingCommandParameter  *commandParameter;
    // 使用トランスポートを保持
    @property (nonatomic) TransportType                 transportType;

@end

@implementation CTAP2ManageCommand

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

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self appHIDCommand] checkUSBHIDConnection];
    }

#pragma mark - Command/subcommand process

    - (void)doRequestHidCtap2Management:(id)parameterRef {
        // PINコード設定処理のパラメーターを保持
        [self setCommandParameter:(FIDOSettingCommandParameter *)parameterRef];
        // トランスポートをUSB HIDに設定
        [self setTransportType:TRANSPORT_HID];
        // CTAPHID_INITから実行
        [self setCommand:[[self commandParameter] command]];
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doResponseHIDCtap2Init {
        // CTAPHID_INIT応答後の処理を実行
        switch ([self command]) {
            case COMMAND_CLIENT_PIN_SET:
            case COMMAND_CLIENT_PIN_CHANGE:
                [self doRequestCommandGetKeyAgreement];
                break;
            case COMMAND_AUTH_RESET:
                [self doRequestCommandAuthReset];
                break;
            default:
                break;
        }
    }

    - (void)doRequestCommandGetKeyAgreement {
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
            case COMMAND_CLIENT_PIN_SET:
                // PIN新規設定処理を続行
                [self doRequestClientPinSet:message];
                break;
            case COMMAND_CLIENT_PIN_CHANGE:
                // PIN変更処理を続行
                [self doRequestClientPinSet:message];
                break;
            default:
                break;
        }
    }

    - (void)doRequestClientPinSet:(NSData *)keyAgreementResponse {
        // メッセージを編集し、サブコマンドを実行
        NSData *request = [self generateClientPinSetRequestWith:keyAgreementResponse];
        if (request == nil) {
            [self commandDidProcess:false message:nil];
            return;
        }
        // コマンドを実行
        [self doRequestCtap2CborCommand:[self command] withData:request];
    }

    - (void)doResponseClientPinSet:(NSData *)message {
        // レスポンスをチェックし、内容がNGであれば処理終了
        if ([self checkStatusCode:message] == false) {
            [self commandDidProcess:false message:nil];
            return;
        }
        // 画面に制御を戻す
        [self commandDidProcess:true message:nil];
    }

    - (void)doRequestCommandAuthReset {
        // リクエスト転送の前に、基板上のボタンを押してもらうように促すメッセージを表示
        [self displayMessage:MSG_CLEAR_PIN_CODE_COMMENT1];
        [self displayMessage:MSG_CLEAR_PIN_CODE_COMMENT2];
        [self displayMessage:MSG_CLEAR_PIN_CODE_COMMENT3];
        // メッセージを編集し、authenticatorResetコマンドを実行
        char commandByte[] = {CTAP2_CMD_RESET};
        NSData *message = [[NSData alloc] initWithBytes:commandByte length:sizeof(commandByte)];
        [self doRequestCtap2CborCommand:COMMAND_AUTH_RESET withData:message];
    }

    - (void)doResponseCommandAuthReset:(NSData *)message {
        // レスポンスをチェックし、内容がNGであれば処理終了
        if ([self checkStatusCode:message] == false) {
            [self commandDidProcess:false message:nil];
            return;
        }
        // 画面に制御を戻す
        [self commandDidProcess:true message:nil];
    }

    - (void)doResponseCtap2Management:(bool)result message:(NSString *)message {
        // 上位クラスに制御を戻す
        [[self delegate] doResponseCtap2Management:result message:message];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時で上位クラスに制御を戻す
        if (success == false) {
            [self doResponseCtap2Management:false message:errorMessage];
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
            case COMMAND_CLIENT_PIN_SET:
            case COMMAND_CLIENT_PIN_CHANGE:
                [self doResponseClientPinSet:response];
                break;
            case COMMAND_AUTH_RESET:
                [self doResponseCommandAuthReset:response];
                break;
            default:
                break;
        }
    }

#pragma mark - Private functions

    - (void)doRequestCtap2CborCommand:(Command)command withData:(NSData *)data {
        // コマンドリクエストを、HIDトランスポート経由で実行
        if ([self transportType] == TRANSPORT_HID) {
            [[self appHIDCommand] doRequestCtap2Command:command withCMD:HID_CMD_CTAPHID_CBOR withData:data];
        }
    }

    - (void)commandDidProcess:(bool)success message:(NSString *)message {
        // コマンド実行完了後の処理
        if ([self transportType] == TRANSPORT_HID) {
            // 上位クラスに制御を戻す
            [self doResponseCtap2Management:success message:message];
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

    - (NSData *)generateClientPinSetRequestWith:(NSData *)message {
        // レスポンスされたCBORを抽出
        NSData *keyAgreementResponse = [ToolCommon extractCBORBytesFrom:message];
        // GetKeyAgreementレスポンスから公開鍵を抽出
        uint8_t *keyAgreement = (uint8_t *)[keyAgreementResponse bytes];
        size_t   keyAgreementSize = [keyAgreementResponse length];
        uint8_t  status_code = ctap2_cbor_decode_get_agreement_key(keyAgreement, keyAgreementSize);
        if (status_code != CTAP1_ERR_SUCCESS) {
            return nil;
        }
        // ECDHキーペアを新規作成し、受領した公開鍵から共通鍵を生成
        if (tool_ecdh_create_shared_secret_key(ctap2_cbor_decode_agreement_pubkey_X(), ctap2_cbor_decode_agreement_pubkey_Y()) == false) {
            return nil;
        }
        // 画面から入力されたPINコードを取得
        NSString *pinNew = [[self commandParameter] pinNew];
        NSString *pinOld = [[self commandParameter] pinOld];

        // pinAuthを生成
        char *pin_new = (char *)[pinNew UTF8String];
        char *pin_old = NULL;
        if ([pinOld length] != 0) {
            pin_old = (char *)[pinOld UTF8String];
        }
        bool change_pin = (pin_old != NULL);
        if (fido_client_pin_generate_pinauth(pin_new, pin_old, change_pin) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Generate pinAuth fail: %s", log_debug_message()];
            return nil;
        }
        // SetPINまたはChangePINリクエストを生成して戻す
        status_code = ctap2_cbor_encode_generate_set_pin_cbor(change_pin, tool_ecdh_public_key_X(), tool_ecdh_public_key_Y(),
            pin_auth(), new_pin_enc(), new_pin_enc_size(), pin_hash_enc());
        if (status_code == CTAP1_ERR_SUCCESS) {
            return [[NSData alloc] initWithBytes:ctap2_cbor_encode_request_bytes()
                                          length:ctap2_cbor_encode_request_bytes_size()];
        } else {
            [[ToolLogFile defaultLogger] errorWithFormat:@"CBOREncoder error: %s", log_debug_message()];
            return nil;
        }
    }

    - (bool)checkStatusCode:(NSData *)responseMessage {
        // レスポンスデータが揃っていない場合はNG
        if (responseMessage == nil || [responseMessage length] == 0) {
            [self displayMessage:MSG_OCCUR_UNKNOWN_ERROR_LEN];
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
                [self displayMessage:[NSString stringWithFormat:MSG_OCCUR_UNKNOWN_ERROR_ST, requestBytes[0]]];
                break;
        }
        return false;
    }

    - (void)displayMessage:(NSString *)message {
        // メッセージを画面表示
        [[self delegate] notifyMessage:message];
    }

@end
