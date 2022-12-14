//
//  BLEPairingCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/19.
//
#import "AppBLECommand.h"
#import "AppCommonMessage.h"
#import "BLEPairingCommand.h"
#import "FIDODefines.h"
#import "ToolCommonFunc.h"

@interface BLEPairingCommand () <AppBLECommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                  delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppBLECommand            *appBLECommand;
    // 実行対象コマンドを保持
    @property (nonatomic) Command                   command;

@end

@implementation BLEPairingCommand

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
        }
        return self;
    }

#pragma mark - Command/subcommand process

    - (void)doResponseBLEPairing:(bool)result message:(NSString *)message {
        // 上位クラスに制御を戻す
        [[self delegate] doResponseBLESettingCommand:result message:message];
    }

#pragma mark - BLE Command/subcommand process

    - (void)doRequestBlePairing {
        // BLEペアリング処理を実行
        [self setCommand:COMMAND_PAIRING];
        [self doRequestCtap2Command:COMMAND_PAIRING withCMD:BLE_CMD_MSG withData:[ToolCommonFunc commandDataForPairingRequest]];
    }

    - (void)doResponseBlePairing:(NSData *)message {
        // 上位クラスに制御を戻す
        [self commandDidProcess:[self checkStatusCode:message] message:nil];
    }

#pragma mark - Call back from AppBLECommand

    - (void)didResponseCommand:(Command)command response:(NSData *)response {
        // 実行コマンドにより処理分岐
        switch (command) {
            case COMMAND_PAIRING:
                [self doResponseBlePairing:response];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、一旦ヘルパークラスに制御を戻す
                [[self appBLECommand] commandDidProcess:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

    - (void)didCompleteCommand:(Command)command success:(bool)success errorMessage:(NSString *)errorMessage {
        // 上位クラスに制御を戻す
        [self doResponseBLEPairing:success message:errorMessage];
    }

#pragma mark - Private functions

    - (void)doRequestCtap2Command:(Command)command withCMD:(uint8_t)cmd withData:(NSData *)data {
        // コマンドリクエストを、BLEトランスポート経由で実行
        [[self appBLECommand] doRequestCommand:command withCMD:cmd withData:data];
    }

    - (void)commandDidProcess:(bool)success message:(NSString *)message {
        // 一旦ヘルパークラスに制御を戻し、BLE切断処理を実行
        [[self appBLECommand] commandDidProcess:success message:message];
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
            default:
                [self displayMessage:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
        return false;
    }

    - (void)displayMessage:(NSString *)message {
        // メッセージを画面表示
        [[self delegate] notifyCommandMessageToMainUI:message];
    }

@end
