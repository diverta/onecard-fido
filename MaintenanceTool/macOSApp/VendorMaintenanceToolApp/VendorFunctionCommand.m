//
//  VendorFunctionCommand.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2023/01/11.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "BootloaderModeCommand.h"
#import "FIDOAttestationCommand.h"
#import "FirmwareResetCommand.h"
#import "ToolCommonFunc.h"
#import "ToolLogFile.h"
#import "VendorFunctionCommand.h"
#import "VendorFunctionWindow.h"
#import "ToolVersionWindow.h"

@implementation VendorFunctionCommandParameter

@end

@interface VendorFunctionCommand () <FIDOAttestationCommandDelegate, BootloaderModeCommandDelegate, FirmwareResetCommandDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) VendorFunctionWindow             *vendorFunctionWindow;
    // ヘルパークラスの参照を保持
    @property (nonatomic) FIDOAttestationCommand           *fidoAttestationCommand;
    @property (nonatomic) BootloaderModeCommand            *bootloaderModeCommand;
    @property (nonatomic) FirmwareResetCommand             *firmwareResetCommand;
    // 処理のパラメーターを保持
    @property (nonatomic) VendorFunctionCommandParameter   *commandParameter;

@end

@implementation VendorFunctionCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // 画面のインスタンスを生成
            [self setVendorFunctionWindow:[[VendorFunctionWindow alloc] initWithWindowNibName:@"VendorFunctionWindow"]];
            // ヘルパークラスのインスタンスを生成
            [self setCommandParameter:[[VendorFunctionCommandParameter alloc] init]];
            [self setFidoAttestationCommand:[[FIDOAttestationCommand alloc] initWithDelegate:self]];
            [self setBootloaderModeCommand:[[BootloaderModeCommand alloc] initWithDelegate:self]];
            [self setFirmwareResetCommand:[[FirmwareResetCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self firmwareResetCommand] isUSBHIDConnected];
    }

    - (void)vendorFunctionWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 重複処理抑止
        if ([[parentWindow sheets] count] > 0) {
            return;
        }
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self vendorFunctionWindow] setParentWindowRef:parentWindow withCommandRef:self withParameterRef:[self commandParameter]];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self vendorFunctionWindow] window];
        VendorFunctionCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf vendorFunctionWindowDidClose:self modalResponse:response];
        }];
    }

#pragma mark - Perform functions

    - (void)vendorFunctionWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self vendorFunctionWindow] close];
    }

    - (void)commandWillPerformVendorFunction {
        // 実行コマンドにより処理分岐
        switch ([[self commandParameter] command]) {
            case COMMAND_INSTALL_ATTESTATION:
                [self installAttestationWillProcess];
                break;
            case COMMAND_REMOVE_ATTESTATION:
                [self removeAttestationWillProcess];
                break;
            case COMMAND_HID_BOOTLOADER_MODE:
                [self bootloaderModeWillProcess];
                break;
            case COMMAND_HID_FIRMWARE_RESET:
                [self firmwareResetWillProcess];
                break;
            default:
                break;
        }
    }

    - (void)installAttestationWillProcess {
        // HIDインターフェース経由で鍵・証明書をインストール-->完了後、FIDOAttestationCommandDidCompleted が呼び出される
        [self notifyProcessStarted];
        [[self fidoAttestationCommand] doRequestInstallAttestation:[self commandParameter]];
    }

    - (void)removeAttestationWillProcess {
        // HIDインターフェース経由で鍵・証明書を削除-->完了後、FIDOAttestationCommandDidCompleted が呼び出される
        [self notifyProcessStarted];
        [[self fidoAttestationCommand] doRequestRemoveAttestation];
    }

    - (void)bootloaderModeWillProcess {
        // HIDインターフェース経由でブートローダーモードに遷移-->完了後、BootloaderModeDidCompleted が呼び出される
        [self notifyProcessStarted];
        [[self bootloaderModeCommand] doRequestBootloaderMode];
    }

    - (void)firmwareResetWillProcess {
        // HIDインターフェース経由でファームウェアをリセット-->完了後、FirmwareResetDidCompleted が呼び出される
        [self notifyProcessStarted];
        [[self firmwareResetCommand] doRequestFirmwareReset];
    }

#pragma mark - Call back from sub command

    - (void)FIDOAttestationCommandDidCompleted:(bool)success message:(NSString *)errorMessage {
        if (success == false) {
            [self notifyErrorMessage:errorMessage];
        }
        [self notifyProcessTerminated:success];
    }

    - (void)BootloaderModeDidCompleted:(bool)success message:(NSString *)errorMessage {
        if (success == false) {
            [self notifyErrorMessage:errorMessage];
        }
        [self notifyProcessTerminated:success];
    }

    - (void)FirmwareResetDidCompleted:(bool)success message:(NSString *)errorMessage {
        if (success == false) {
            [self notifyErrorMessage:errorMessage];
        }
        [self notifyProcessTerminated:success];
    }

#pragma mark - Private common methods

    - (void)notifyProcessStarted {
        // コマンド開始メッセージをログファイルに出力
        NSString *startMsg = [NSString stringWithFormat:MSG_FORMAT_START_MESSAGE, [[self commandParameter] commandName]];
        [[ToolLogFile defaultLogger] info:startMsg];
    }

    - (void)notifyErrorMessage:(NSString *)message {
        // エラーメッセージをログファイルに出力（出力前に改行文字を削除）
        NSString *logMessage = [message stringByReplacingOccurrencesOfString:@"\n" withString:@""];
        [[ToolLogFile defaultLogger] error:logMessage];
        // 戻り先画面に表示させるためのエラーメッセージを保持
        [[self commandParameter] setCommandErrorMessage:message];
    }

    - (void)notifyProcessTerminated:(bool)success {
        // コマンド終了メッセージを生成
        NSString *endMsg = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE, [[self commandParameter] commandName], success ? MSG_SUCCESS : MSG_FAILURE];
        if (success == false) {
            // コマンド異常終了メッセージをログ出力
            [[ToolLogFile defaultLogger] error:endMsg];
        } else {
            // コマンド正常終了メッセージをログ出力
            [[ToolLogFile defaultLogger] info:endMsg];
        }
        // 画面に制御を戻す
        [[self commandParameter] setCommandSuccess:success];
        [[self vendorFunctionWindow] vendorFunctionCommandDidProcess];
    }

@end
