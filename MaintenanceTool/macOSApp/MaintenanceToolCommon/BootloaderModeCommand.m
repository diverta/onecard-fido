//
//  BootloaderModeCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/01/11.
//
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "FIDODefines.h"
#import "BootloaderModeCommand.h"
#import "ToolCommonFunc.h"

@interface BootloaderModeCommand () <AppHIDCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                  delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand            *appHIDCommand;
    // 実行対象コマンドを保持
    @property (nonatomic) Command                   command;
    // 使用トランスポートを保持
    @property (nonatomic) TransportType             transportType;
    // HID接続断検知を行うかどうかを保持
    @property(nonatomic) bool                       needNotifyDetectDisconnect;

@end

@implementation BootloaderModeCommand

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

    - (void)doRequestBootloaderMode {
        // トランスポートをUSB HIDに設定
        [self setTransportType:TRANSPORT_HID];
        // CTAPHID_INITから実行
        [self setCommand:COMMAND_HID_BOOTLOADER_MODE];
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doResponseHIDCtap2Init {
        // CTAPHID_INIT応答後の処理を実行
        if ([self command] == COMMAND_HID_BOOTLOADER_MODE) {
            [self doRequestHidBootloaderMode];
        }
    }

    - (void)doRequestHidBootloaderMode {
        // HID接続検知フラグをクリア
        [self setNeedNotifyDetectDisconnect:false];
        // メッセージを編集し、コマンド 0xC6 を実行
        uint8_t cmd = MNT_COMMAND_BASE | 0x80;
        [[self appHIDCommand] doRequestCtap2Command:COMMAND_HID_BOOTLOADER_MODE withCMD:cmd withData:[ToolCommonFunc commandDataForChangeToBootloaderMode]];
    }

    - (void)doResponseHidBootloaderMode:(NSData *)response CMD:(uint8_t)cmd {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *requestBytes = (uint8_t *)[response bytes];
        if (requestBytes[0] != CTAP1_ERR_SUCCESS) {
            // エラーの場合は画面に制御を戻す
            [self commandDidProcess:false message:MSG_DFU_TARGET_NOT_BOOTLOADER_MODE];
            return;
        }
        // ブートローダーモード遷移コマンド成功時
        if (cmd != HID_CMD_UNKNOWN_ERROR) {
            // 接続断まで待機 --> didDetectRemoval が呼び出される
            [self setNeedNotifyDetectDisconnect:true];

        } else {
            // ブートローダーモード遷移コマンド失敗時は、即時で制御を戻す
            [self commandDidProcess:false message:MSG_DFU_TARGET_NOT_BOOTLOADER_MODE];
        }
    }

    - (void)commandDidProcess:(bool)result message:(NSString *)message {
        // 上位クラスに制御を戻す
        [[self delegate] BootloaderModeDidCompleted:result message:message];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
        // HID接続断を検知したら、上位クラスに制御を戻す
        if ([self needNotifyDetectDisconnect]) {
            [self setNeedNotifyDetectDisconnect:false];
            [self commandDidProcess:true message:nil];
        }
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        if ([self command] == COMMAND_HID_BOOTLOADER_MODE) {
            if (success == false) {
                // 即時で上位クラスに制御を戻す
                [self commandDidProcess:false message:errorMessage];
                return;
            }
            // 実行コマンドにより処理分岐
            if (command == COMMAND_HID_CTAP2_INIT) {
                [self doResponseHIDCtap2Init];
            }
            if (command == COMMAND_HID_BOOTLOADER_MODE) {
                [self doResponseHidBootloaderMode:response CMD:cmd];
            }
        }
    }

@end
