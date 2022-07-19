//
//  CTAP2ManageCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/19.
//
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "CTAP2ManageCommand.h"
#import "FIDODefines.h"
#import "FIDOSettingCommand.h"

@interface CTAP2ManageCommand () <AppHIDCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                      delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;
    // 実行対象コマンド／サブコマンドを保持
    @property (nonatomic) Command                       command;
    @property (nonatomic) uint8_t                       cborCommand;
    @property (nonatomic) uint8_t                       subCommand;
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
            case COMMAND_AUTH_RESET:
                [self doRequestCommandAuthReset];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self commandDidProcess:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
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

    - (void)didResponseCommand:(Command)command response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
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
            case COMMAND_AUTH_RESET:
                [self doResponseCommandAuthReset:response];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self doResponseCtap2Management:false message:MSG_OCCUR_UNKNOWN_ERROR];
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
