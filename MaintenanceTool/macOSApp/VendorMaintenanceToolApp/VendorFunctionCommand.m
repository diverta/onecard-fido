//
//  VendorFunctionCommand.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2023/01/11.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "AppHIDCommand.h"
#import "ToolCommonFunc.h"
#import "ToolLogFile.h"
#import "VendorFunctionCommand.h"
#import "VendorFunctionWindow.h"
#import "ToolVersionWindow.h"

@implementation VendorFunctionCommandParameter

@end

@interface VendorFunctionCommand () <AppHIDCommandDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) VendorFunctionWindow             *vendorFunctionWindow;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                    *appHIDCommand;
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
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self appHIDCommand] checkUSBHIDConnection];
    }

    - (void)vendorFunctionWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
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
        // 実行コマンドにより処理分岐
        switch ([[self commandParameter] command]) {
            case COMMAND_INSTALL_ATTESTATION:
                // TODO: FIDO鍵・証明書インストール
                break;
            case COMMAND_REMOVE_ATTESTATION:
                // TODO: FIDO鍵・証明書削除
                break;
            case COMMAND_HID_BOOTLOADER_MODE:
                // TODO: ブートローダーモード遷移
                break;
            case COMMAND_HID_FIRMWARE_RESET:
                // TODO: ファームウェアリセット
                break;
            default:
                break;
        }
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        switch ([[self commandParameter] command]) {
            case COMMAND_INSTALL_ATTESTATION:
            case COMMAND_REMOVE_ATTESTATION:
            case COMMAND_HID_BOOTLOADER_MODE:
            case COMMAND_HID_FIRMWARE_RESET:
                break;
            default:
                return;
        }
        if (success == false) {
            // 即時でアプリケーションに制御を戻す
            [self notifyCommandTerminated:[self commandName] message:errorMessage success:success fromWindow:[self parentWindow]];
            return;
        }
        // 実行コマンドにより処理分岐
        switch ([[self commandParameter] command]) {
            case COMMAND_INSTALL_ATTESTATION:
            case COMMAND_REMOVE_ATTESTATION:
            case COMMAND_HID_BOOTLOADER_MODE:
            case COMMAND_HID_FIRMWARE_RESET:
                break;
            default:
                break;
        }
    }

@end
