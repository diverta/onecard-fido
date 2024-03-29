//
//  VendorFunctionWindow.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2023/01/11.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "FIDOAttestationWindow.h"
#import "ToolCommonFunc.h"
#import "ToolPopupWindow.h"
#import "ToolProcessingWindow.h"
#import "UtilityCommand.h"
#import "VendorFunctionCommand.h"
#import "VendorFunctionWindow.h"

@interface VendorFunctionWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // 子画面の参照を保持
    @property (nonatomic) FIDOAttestationWindow            *fidoAttestationWindow;
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
        // 子画面のインスタンスを生成
        [self setFidoAttestationWindow:[[FIDOAttestationWindow alloc] initWithWindowNibName:@"FIDOAttestationWindow"]];
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
        // FIDO鍵・証明書設定画面をポップアップ表示
        [[self fidoAttestationWindow] setParentWindowRef:[self window] withCommandRef:[self vendorFunctionCommandRef] withParameterRef:[self commandParameterRef]];
        NSWindow *dialog = [[self fidoAttestationWindow] window];
        VendorFunctionWindow * __weak weakSelf = self;
        [[self window] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf fidoAttestationWindowDidClose:self modalResponse:response];
        }];
    }

    - (void)fidoAttestationWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self fidoAttestationWindow] close];
        if (modalResponse != NSModalResponseOK) {
            return;
        }
        // OKボタンクリックの場合は、FIDO鍵・証明書をインストール
        [[self commandParameterRef] setCommand:COMMAND_INSTALL_ATTESTATION];
        [[self commandParameterRef] setCommandName:PROCESS_NAME_INSTALL_ATTESTATION];
        [self commandWillPerformVendorFunction];
    }

    - (IBAction)buttonRemoveAttestationDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 処理開始前に確認
        [[ToolPopupWindow defaultWindow] criticalPrompt:MSG_ERASE_SKEY_CERT informativeText:MSG_PROMPT_ERASE_SKEY_CERT
                                             withObject:self forSelector:@selector(resumeRemoveAttestation) parentWindow:[self window]];
    }

    - (void)resumeRemoveAttestation {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // 鍵・証明書を削除
        [[self commandParameterRef] setCommand:COMMAND_REMOVE_ATTESTATION];
        [[self commandParameterRef] setCommandName:PROCESS_NAME_REMOVE_ATTESTATION];
        [self commandWillPerformVendorFunction];
    }

    - (IBAction)buttonBootloaderModeDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // ブートローダーモードに遷移するかどうかのプロンプトを表示
        [[ToolPopupWindow defaultWindow] criticalPrompt:MSG_CHANGE_TO_BOOTLOADER_MODE informativeText:MSG_PROMPT_CHANGE_TO_BOOTLOADER_MODE
                                             withObject:self forSelector:@selector(resumeBootloaderMode) parentWindow:[self window]];
    }

    - (void)resumeBootloaderMode {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // ブートローダーモードに遷移
        [[self commandParameterRef] setCommand:COMMAND_HID_BOOTLOADER_MODE];
        [[self commandParameterRef] setCommandName:PROCESS_NAME_BOOT_LOADER_MODE];
        [self commandWillPerformVendorFunction];
    }

    - (IBAction)buttonFirmwareResetDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 認証器のファームウェアを再起動するかどうかのプロンプトを表示
        [[ToolPopupWindow defaultWindow] criticalPrompt:MSG_FIRMWARE_RESET informativeText:MSG_PROMPT_FIRMWARE_RESET
                                                 withObject:self forSelector:@selector(resumeFirmwareReset) parentWindow:[self window]];
    }

    - (void)resumeFirmwareReset {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
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
