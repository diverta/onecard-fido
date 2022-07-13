//
//  CTAP2HcheckCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/13.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "AppHIDCommand.h"
#import "CTAP2HcheckCommand.h"
#import "FIDODefines.h"
#import "ToolCommon.h"

@interface CTAP2HcheckCommand () <AppHIDCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                  delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand            *appHIDCommand;
    // 実行対象コマンドを保持
    @property (nonatomic) Command                   command;
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

    - (void)doRequestHidCtap2HealthCheck {
        // トランスポートをUSB HIDに設定
        [self setTransportType:TRANSPORT_HID];
        // CTAPHID_INITから実行
        [self setCommand:COMMAND_TEST_MAKE_CREDENTIAL];
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doRequestCommandGetKeyAgreement {
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
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self doResponseCtap2HealthCheck:false message:nil];
                break;
        }
    }

#pragma mark - Private functions

@end
