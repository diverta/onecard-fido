//
//  USBDFUTransferCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/19.
//
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "DFUCommand.h"
#import "FIDODefines.h"
#import "USBDFUACMCommand.h"
#import "USBDFUTransferCommand.h"
#import "ToolLogFile.h"

@interface USBDFUTransferCommand () <AppHIDCommandDelegate, USBDFUACMCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                      delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;
    @property (nonatomic) USBDFUACMCommand             *acmCommand;
    // DFU処理のパラメーターを保持
    @property (nonatomic) DFUCommandParameter          *commandParameter;

@end

@implementation USBDFUTransferCommand

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
            [self setAcmCommand:[[USBDFUACMCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (void)terminateTransferCommand:(bool)success {
        // 上位クラスに制御を戻す
        [[self delegate] transferCommandDidTerminate:success];
    }

#pragma mark - Change to bootloader mode

    - (void)invokeTransferWithParamRef:(id)ref {
        // DFU処理のパラメーターを保持
        [self setCommandParameter:(DFUCommandParameter *)ref];
        // CTAPHID_INITから実行
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doResponseHIDCtap2Init {
        // CTAPHID_INIT応答後の処理を実行
        [self doRequestHidBootloaderMode];
    }

    - (void)doRequestHidBootloaderMode {
        // メッセージを編集し、コマンド 0xC6 を実行
        NSData *commandData = [[NSData alloc] init];
        [[self appHIDCommand] doRequestCtap2Command:COMMAND_HID_BOOTLOADER_MODE withCMD:HID_CMD_BOOTLOADER_MODE withData:commandData];
    }

    - (void)doResponseHidBootloaderMode:(uint8_t)cmd response:(NSData *)response {
        // ブートローダーモード遷移コマンド成功時
        if (cmd == HID_CMD_BOOTLOADER_MODE) {
            // 処理ステータスを設定 --> USB切断検知により処理続行
            [[self commandParameter] setDfuStatus:DFU_ST_TO_BOOTLOADER_MODE];

        } else {
            // ブートローダーモード遷移コマンド失敗時は、即時で制御を戻す
            [[self delegate] notifyErrorMessage:MSG_DFU_TARGET_NOT_BOOTLOADER_MODE];
            [self terminateTransferCommand:false];
        }
    }

#pragma mark - DFU transfer main process

    - (void)performTransferProcess {
        // TODO: 仮の実装です。
        [NSThread sleepForTimeInterval:2.0];
        [[self acmCommand] closeACMConnection];
        [self terminateTransferCommand:true];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
        if ([[self commandParameter] dfuStatus] != DFU_ST_TO_BOOTLOADER_MODE) {
            return;
        }
        // ステータスをクリア
        [[self commandParameter] setDfuStatus:DFU_ST_NONE];
        // USB DFUに必要なACM接続を確立
        [[self acmCommand] establishACMConnection];
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時で上位クラスに制御を戻す
        if (success == false) {
            [[ToolLogFile defaultLogger] error:errorMessage];
            [self terminateTransferCommand:false];
            return;
        }
        // 実行コマンドにより処理分岐
        switch (command) {
            case COMMAND_HID_CTAP2_INIT:
                [self doResponseHIDCtap2Init];
                break;
            case COMMAND_HID_BOOTLOADER_MODE:
                [self doResponseHidBootloaderMode:cmd response:response];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self terminateTransferCommand:false];
                break;
        }
    }

#pragma mark - Call back from USBDFUACMCommand

    - (void)didEstablishACMConnection:(bool)success {
        [self performTransferProcess];
    }

@end
