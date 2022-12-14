//
//  EraseBondsCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/12/14.
//
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "EraseBondsCommand.h"
#import "FIDODefines.h"
#import "ToolCommonFunc.h"
#import "ToolLogFile.h"

@interface EraseBondsCommand () <AppHIDCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                  delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand            *appHIDCommand;
    // 実行対象コマンドを保持
    @property (nonatomic) Command                   command;

@end

@implementation EraseBondsCommand

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

    - (void)doRequestHidEraseBonds {
        // CTAPHID_INITから実行
        [self setCommand:COMMAND_ERASE_BONDS];
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doResponseHIDCtap2Init {
        // CTAPHID_INIT応答後の処理を実行
        [self doRequestEraseBondsCommand];
    }

    - (void)doRequestEraseBondsCommand {
        // メッセージを編集し、コマンド 0x46 を実行
        uint8_t cmd = MNT_COMMAND_BASE | 0x80;
        [[self appHIDCommand] doRequestCtap2Command:COMMAND_ERASE_BONDS withCMD:cmd withData:[ToolCommonFunc commandDataForEraseBondingData]];
    }

    - (void)doResponseEraseBondsCommand:(NSData *)response {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *responseBytes = (uint8_t *)[response bytes];
        if (responseBytes[0] != CTAP1_ERR_SUCCESS) {
            // エラーの場合は上位クラスに制御を戻す
            [self doResponseHidEraseBonds:false message:MSG_OCCUR_UNKNOWN_ERROR];
            return;
        }
        // 上位クラスに制御を戻す
        [self doResponseHidEraseBonds:true message:nil];
    }

    - (void)doResponseHidEraseBonds:(bool)success message:(NSString *)message {
        // 処理失敗時はログを出力
        if (success == false && message != nil) {
            [[ToolLogFile defaultLogger] error:message];
        }
        // 上位クラスに制御を戻す
        [[self delegate] doResponseBLESettingCommand:success message:message];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時で上位クラスに制御を戻す
        if (success == false) {
            [self doResponseHidEraseBonds:false message:errorMessage];
            return;
        }
        // 実行コマンドにより処理分岐
        switch (command) {
            case COMMAND_HID_CTAP2_INIT:
                [self doResponseHIDCtap2Init];
                break;
            case COMMAND_ERASE_BONDS:
                [self doResponseEraseBondsCommand:response];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self doResponseHidEraseBonds:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

@end
