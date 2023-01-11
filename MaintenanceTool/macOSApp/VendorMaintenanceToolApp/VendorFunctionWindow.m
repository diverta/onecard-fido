//
//  VendorFunctionWindow.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2023/01/11.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "ToolCommonFunc.h"
#import "ToolPopupWindow.h"
#import "ToolProcessingWindow.h"
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
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 実行コマンドを設定して画面を閉じる
        [[self commandParameterRef] setCommand:COMMAND_INSTALL_ATTESTATION];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonRemoveAttestationDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 実行コマンドを設定して画面を閉じる
        [[self commandParameterRef] setCommand:COMMAND_REMOVE_ATTESTATION];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonBootloaderModeDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 実行コマンドを設定して画面を閉じる
        [[self commandParameterRef] setCommand:COMMAND_HID_BOOTLOADER_MODE];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonFirmwareResetDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 認証器のファームウェアを再起動
        [[self commandParameterRef] setCommand:COMMAND_HID_FIRMWARE_RESET];
        [[self commandParameterRef] setCommandName:PROCESS_NAME_FIRMWARE_RESET];
        [self commandWillPerformVendorFunction];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合は処理中止
        VendorFunctionCommand *command = (VendorFunctionCommand *)[self vendorFunctionCommandRef];
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:[self window] connected:[command isUSBHIDConnected]];
    }

    - (void)commandWillPerformVendorFunction {
        // 進捗画面を表示
        [[ToolProcessingWindow defaultWindow] windowWillOpenWithCommandRef:self withParentWindow:[self window]];
        // ベンダー向け機能を実行
        VendorFunctionCommand *command = (VendorFunctionCommand *)[self vendorFunctionCommandRef];
        [command commandWillPerformVendorFunction];
    }

    - (void)vendorFunctionCommandDidProcess {
        // 進捗画面を閉じる
        [[ToolProcessingWindow defaultWindow] windowWillCloseForTarget:self forSelector:@selector(toolProcessingWindowDidClose)];
    }

    - (void)toolProcessingWindowDidClose {
        // ポップアップ表示させるメッセージを編集
        NSString *message = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE, [[self commandParameterRef] commandName],
                             [[self commandParameterRef] commandSuccess] ? MSG_SUCCESS:MSG_FAILURE];
        // 処理終了メッセージをポップアップ表示
        if ([[self commandParameterRef] commandSuccess]) {
            [[ToolPopupWindow defaultWindow] informational:message informativeText:nil withObject:nil forSelector:nil parentWindow:[self window]];
        } else {
            [[ToolPopupWindow defaultWindow] critical:message informativeText:[[self commandParameterRef] commandErrorMessage] withObject:nil forSelector:nil parentWindow:[self window]];
        }
    }

@end
