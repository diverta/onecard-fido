//
//  VendorFunctionWindow.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2023/01/11.
//
#import "AppDefine.h"
#import "UtilityCommand.h"
#import "VendorFunctionCommand.h"
#import "VendorFunctionWindow.h"

@interface VendorFunctionWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) id                          vendorFunctionCommandRef;
    // 処理パラメーターを保持
    @property (nonatomic) VendorFunctionCommandParameter   *commandParameterRef;

@end

@implementation VendorFunctionWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
        // コマンドクラスの参照を保持
        [self setVendorFunctionCommandRef:commandRef];
        // ヘルスチェック処理のパラメーターを保持
        [self setCommandParameterRef:parameterRef];
        // パラメーターを初期化
        [[self commandParameterRef] setCommand:COMMAND_NONE];
    }

    - (IBAction)buttonInstallAttestationDidPress:(id)sender {
        // 実行コマンドを設定して画面を閉じる
        [[self commandParameterRef] setCommand:COMMAND_INSTALL_ATTESTATION];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonRemoveAttestationDidPress:(id)sender {
        // 実行コマンドを設定して画面を閉じる
        [[self commandParameterRef] setCommand:COMMAND_REMOVE_ATTESTATION];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonBootloaderModeDidPress:(id)sender {
        // 実行コマンドを設定して画面を閉じる
        [[self commandParameterRef] setCommand:COMMAND_HID_BOOTLOADER_MODE];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonFirmwareResetDidPress:(id)sender {
        // 実行コマンドを設定して画面を閉じる
        [[self commandParameterRef] setCommand:COMMAND_HID_FIRMWARE_RESET];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

@end
