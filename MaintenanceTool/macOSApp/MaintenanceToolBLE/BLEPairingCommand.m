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

    - (void)doRequestBLEPairing {
        // BLEペアリング処理を実行
        [self setCommand:COMMAND_PAIRING];
        [[self appBLECommand] doRequestCommand:COMMAND_PAIRING withCMD:BLE_CMD_MSG withData:[ToolCommonFunc commandDataForPairingRequest]];
    }

    - (void)doResponseBlePairingCommand:(NSData *)response {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *responseBytes = (uint8_t *)[response bytes];
        if (responseBytes[0] != CTAP1_ERR_SUCCESS) {
            // エラーの場合はヘルパークラスに制御を戻す
            [[self appBLECommand] commandDidProcess:false message:MSG_OCCUR_UNKNOWN_ERROR];
            return;
        }
        // 一旦ヘルパークラスに制御を戻し、BLE切断処理を実行
        [[self appBLECommand] commandDidProcess:true message:nil];
    }

    - (void)doResponseBLEPairing:(bool)success message:(NSString *)message {
        // 上位クラスに制御を戻す
        [[self delegate] doResponseBLESettingCommand:success message:message];
    }

#pragma mark - Call back from AppBLECommand

    - (void)didResponseCommand:(Command)command response:(NSData *)response {
        if (command == COMMAND_PAIRING) {
            [self doResponseBlePairingCommand:response];
        }
    }

    - (void)didCompleteCommand:(Command)command success:(bool)success errorMessage:(NSString *)errorMessage {
        // 上位クラスに制御を戻す
        [self doResponseBLEPairing:success message:errorMessage];
    }

@end
