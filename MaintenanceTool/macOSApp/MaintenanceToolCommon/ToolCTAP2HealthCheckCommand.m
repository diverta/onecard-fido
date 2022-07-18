//
//  ToolCTAP2HealthCheckCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/02.
//
#import <Foundation/Foundation.h>

#import "AppCommonMessage.h"
#import "ToolCommon.h"
#import "ToolContext.h"
#import "ToolCTAP2HealthCheckCommand.h"
#import "FIDODefines.h"
#import "CBORDecoder.h"
#import "CBOREncoder.h"
#import "debug_log.h"
#import "ToolLogFile.h"

@interface ToolCTAP2HealthCheckCommand ()

    @property (nonatomic) TransportType       transportType;
    @property (nonatomic) ToolBLECommand     *toolBLECommand;
    @property (nonatomic) ToolHIDCommand     *toolHIDCommand;

    // 実行対象コマンド／サブコマンドを保持
    @property (nonatomic) Command   command;
    @property (nonatomic) uint8_t   cborCommand;
    @property (nonatomic) uint8_t   subCommand;

@end

@implementation ToolCTAP2HealthCheckCommand

    - (id)init {
        self = [super init];
        return self;
    }

    - (void)setTransportParam:(TransportType)type
               toolBLECommand:(ToolBLECommand *)ble toolHIDCommand:(ToolHIDCommand *)hid {
        [self setTransportType:type];
        [self setToolBLECommand:ble];
        [self setToolHIDCommand:hid];
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

#pragma mark - Command/subcommand process

    - (void)doCTAP2Request:(Command)command {
        // 実行対象コマンドを退避
        [self setCommand:command];
        switch ([self command]) {
            case COMMAND_CLIENT_PIN_SET:
            case COMMAND_CLIENT_PIN_CHANGE:
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
            [[self toolHIDCommand] commandDidProcess:[self command] result:result message:message];
        }
    }

    - (NSData *)extractCBORBytesFrom:(NSData *)responseMessage {
        // CBORバイト配列（レスポンスの２バイト目以降）を抽出
        size_t cborLength = [responseMessage length] - 1;
        NSData *cborBytes = [responseMessage subdataWithRange:NSMakeRange(1, cborLength)];
        return cborBytes;
    }

@end
