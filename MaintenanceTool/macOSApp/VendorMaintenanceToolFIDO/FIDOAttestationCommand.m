//
//  FIDOAttestationCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/01/11.
//
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "FIDODefines.h"
#import "FIDOAttestationCommand.h"
#import "ToolCommonFunc.h"
#import "VendorFunctionCommand.h"

@interface FIDOAttestationCommand () <AppHIDCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                          delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                    *appHIDCommand;
    // 実行対象コマンドを保持
    @property (nonatomic) Command                           command;
    // 使用トランスポートを保持
    @property (nonatomic) TransportType                     transportType;
    // 処理のパラメーターを保持
    @property (nonatomic) VendorFunctionCommandParameter   *commandParameter;

@end

@implementation FIDOAttestationCommand

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

    - (void)doRequestInstallAttestation:(id)commandParameterRef {
        // トランスポートをUSB HIDに設定
        [self setTransportType:TRANSPORT_HID];
        // パラメーターを保持
        [self setCommandParameter:(VendorFunctionCommandParameter *)commandParameterRef];
        // CTAPHID_INITから実行
        [self setCommand:COMMAND_INSTALL_ATTESTATION];
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doRequestRemoveAttestation {
        // トランスポートをUSB HIDに設定
        [self setTransportType:TRANSPORT_HID];
        // CTAPHID_INITから実行
        [self setCommand:COMMAND_REMOVE_ATTESTATION];
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doResponseHIDCtap2Init {
        // CTAPHID_INIT応答後の処理を実行
        if ([self command] == COMMAND_INSTALL_ATTESTATION) {
            [self doRequestHidInstallAttestation];
        }
        if ([self command] == COMMAND_REMOVE_ATTESTATION) {
            [self doRequestHidRemoveAttestation];
        }
    }

    - (void)doRequestHidInstallAttestation {
        // メッセージを編集し、コマンド 0xC8 を実行
        NSData *data = [[NSData alloc] init];
        [[self appHIDCommand] doRequestCtap2Command:COMMAND_INSTALL_ATTESTATION withCMD:HID_CMD_INSTALL_ATTESTATION withData:data];
    }

    - (void)doResponseHidInstallAttestation:(NSData *)response {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *responseBytes = (uint8_t *)[response bytes];
        if (responseBytes[0] != CTAP1_ERR_SUCCESS) {
            // エラーの場合はヘルパークラスに制御を戻す
            NSString *message = [NSString stringWithFormat:MSG_OCCUR_UNKNOWN_ERROR_ST, responseBytes[0]];
            [self commandDidProcess:false message:message];
            return;
        }
        [self commandDidProcess:true message:nil];
    }

    - (void)doRequestHidRemoveAttestation {
        // メッセージを編集し、コマンド 0xC9 を実行
        NSData *data = [[NSData alloc] init];
        [[self appHIDCommand] doRequestCtap2Command:COMMAND_REMOVE_ATTESTATION withCMD:HID_CMD_RESET_ATTESTATION withData:data];
    }

    - (void)doResponseHidRemoveAttestation:(NSData *)response {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *responseBytes = (uint8_t *)[response bytes];
        if (responseBytes[0] != CTAP1_ERR_SUCCESS) {
            // エラーの場合はヘルパークラスに制御を戻す
            NSString *message = [NSString stringWithFormat:MSG_OCCUR_UNKNOWN_ERROR_ST, responseBytes[0]];
            [self commandDidProcess:false message:message];
            return;
        }
        [self commandDidProcess:true message:nil];
    }

    - (void)commandDidProcess:(bool)result message:(NSString *)message {
        // 上位クラスに制御を戻す
        [[self delegate] FIDOAttestationCommandDidCompleted:result message:message];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        if ([self command] == COMMAND_INSTALL_ATTESTATION || [self command] == COMMAND_REMOVE_ATTESTATION) {
            if (success == false) {
                // 即時で上位クラスに制御を戻す
                [self commandDidProcess:false message:errorMessage];
                return;
            }
            // 実行コマンドにより処理分岐
            if (command == COMMAND_HID_CTAP2_INIT) {
                [self doResponseHIDCtap2Init];
            }
            if (command == COMMAND_INSTALL_ATTESTATION) {
                [self doResponseHidInstallAttestation:response];
            }
            if (command == COMMAND_REMOVE_ATTESTATION) {
                [self doResponseHidRemoveAttestation:response];
            }
        }
    }

@end
