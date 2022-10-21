//
//  FirmwareResetCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/21.
//
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "FIDODefines.h"
#import "FirmwareResetCommand.h"

@interface FirmwareResetCommand () <AppHIDCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                  delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand            *appHIDCommand;
    // 実行対象コマンドを保持
    @property (nonatomic) Command                   command;
    // 使用トランスポートを保持
    @property (nonatomic) TransportType             transportType;
    // HID再接続検知を行うかどうかを保持
    @property(nonatomic) bool                       needNotifyDetectConnect;

@end

@implementation FirmwareResetCommand

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

    - (void)doRequestFirmwareReset {
        // トランスポートをUSB HIDに設定
        [self setTransportType:TRANSPORT_HID];
        // CTAPHID_INITから実行
        [self setCommand:COMMAND_HID_FIRMWARE_RESET];
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doResponseHIDCtap2Init {
        // CTAPHID_INIT応答後の処理を実行
        switch ([self command]) {
            case COMMAND_HID_FIRMWARE_RESET:
                [self doRequestHidFirmwareReset];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self commandDidProcess:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

    - (void)doRequestHidFirmwareReset {
        // HID接続検知フラグをクリア
        [self setNeedNotifyDetectConnect:false];
        // コマンド 0xC7 を実行（メッセージはブランクとする）
        NSData *message = [[NSData alloc] init];
        [[self appHIDCommand] doRequestCtap2Command:[self command] withCMD:HID_CMD_FIRMWARE_RESET withData:message];
    }

    - (void)doResponseHidFirmwareReset:(NSData *)response {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認し、エラーの場合は上位クラスに制御を戻す
        uint8_t *requestBytes = (uint8_t *)[response bytes];
        if (requestBytes[0] != CTAP1_ERR_SUCCESS) {
            [self commandDidProcess:false message:MSG_FIRMWARE_RESET_UNSUPP];
        } else {
            // 再接続まで待機 --> completedResetFirmware が呼び出される
            [self setNeedNotifyDetectConnect:true];
        }
    }

    - (void)commandDidProcess:(bool)result message:(NSString *)message {
        // 上位クラスに制御を戻す
        [[self delegate] FirmwareResetDidCompleted:result message:message];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
        // HID接続を検知したら、上位クラスに制御を戻す
        if ([self needNotifyDetectConnect]) {
            [self setNeedNotifyDetectConnect:false];
            [self commandDidProcess:true message:nil];
        }
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時で上位クラスに制御を戻す
        if (success == false) {
            [self commandDidProcess:false message:errorMessage];
            return;
        }
        // 実行コマンドにより処理分岐
        switch (command) {
            case COMMAND_HID_CTAP2_INIT:
                [self doResponseHIDCtap2Init];
                break;
            case COMMAND_HID_FIRMWARE_RESET:
                [self doResponseHidFirmwareReset:response];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self commandDidProcess:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

@end
