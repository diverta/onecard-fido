//
//  ToolAppCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/14.
//
#import "AppCommonMessage.h"
#import "FIDODefines.h"
#import "ToolAppCommand.h"
#import "ToolBLEDFUCommand.h"
#import "ToolCommonMessage.h"
#import "ToolUSBDFUCommand.h"
#import "ToolPGPCommand.h"
#import "ToolHIDCommand.h"
#import "ToolLogFile.h"
#import "ToolPIVCommand.h"
#import "ToolPopupWindow.h"
#import "ToolDFUCommand.h"

@interface ToolAppCommand () <ToolHIDCommandDelegate, ToolBLEDFUCommandDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow              *parentWindow;
    // 下位クラスの参照を保持
    @property (nonatomic) ToolHIDCommand        *toolHIDCommand;
    @property (nonatomic) ToolBLEDFUCommand     *toolBLEDFUCommand;
    @property (nonatomic) ToolUSBDFUCommand     *toolUSBDFUCommand;
    @property (nonatomic) ToolPIVCommand        *toolPIVCommand;
    @property (nonatomic) ToolDFUCommand        *toolDFUCommand;
    @property (nonatomic) ToolPGPCommand        *toolPGPCommand;
    // 処理機能名称を保持
    @property (nonatomic) NSString *processNameOfCommand;
    // 実行するコマンドの種別を保持
    @property (nonatomic) Command   command;

@end

@implementation ToolAppCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolAppCommandDelegate>)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
            // コマンドクラスの初期化
            [self setToolHIDCommand:[[ToolHIDCommand alloc] initWithDelegate:self]];
            // PIV機能の初期設定
            [self setToolPIVCommand:[[ToolPIVCommand alloc] initWithDelegate:self]];
            // DFU機能の初期設定
            [self setToolDFUCommand:[[ToolDFUCommand alloc] initWithDelegate:self]];
            [self setToolBLEDFUCommand:[[ToolBLEDFUCommand alloc] initWithDelegate:self]];
            [self setToolUSBDFUCommand:[[ToolUSBDFUCommand alloc] initWithDelegate:self]];
            // GPG機能の初期設定
            [self setToolPGPCommand:[[ToolPGPCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

#pragma mark - Public methods

    - (void)doCommandBLMode:(NSWindow *)parentWindow {
        // ブートローダーモード遷移
        [self doHIDCommand:COMMAND_HID_BOOTLOADER_MODE sender:nil parentWindow:parentWindow];
    }

    - (void)doCommandFirmwareResetForCommandRef:(id)ref {
        // 認証器にファームウェアリセットを要求
        [self doRequestForResetFirmware:COMMAND_HID_FIRMWARE_RESET forCommandRef:ref];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        return [[self toolHIDCommand] checkUSBHIDConnection];
    }

#pragma mark - For HID connection check

    - (void)doHIDCommand:(Command)command sender:(id)sender parentWindow:(NSWindow *)parentWindow  {
        // 実行コマンドを退避
        [self setCommand:command];
        // ボタン／メニューを非活性化
        [[self delegate] disableUserInterface];
        // HID接続状態をチェック
        if ([self checkUSBHIDConnection] == false) {
            // エラーメッセージをポップアップ表示-->ボタンを活性化
            [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_PROMPT_USB_PORT_SET informativeText:nil
                                           withObject:self forSelector:@selector(enableUserInterface) parentWindow:parentWindow];
            return;
        }
        // コマンドごとの後続処理
        switch (command) {
            case COMMAND_HID_BOOTLOADER_MODE:
                // ブートローダーモード遷移
                [[ToolPopupWindow defaultWindow] criticalPrompt:MSG_BOOT_LOADER_MODE informativeText:MSG_PROMPT_BOOT_LOADER_MODE
                                                     withObject:self forSelector:@selector(resumeHIDCommand) parentWindow:parentWindow];
                break;
            case COMMAND_OPEN_WINDOW_PIVPARAM:
                // PIV機能設定画面を表示
                [[self toolPIVCommand] commandWillOpenPreferenceWindowWithParent:parentWindow];
                break;
            case COMMAND_OPEN_WINDOW_PGPPARAM:
                // OpenPGP機能設定画面を表示
                [[self toolPGPCommand] commandWillOpenPreferenceWindowWithParent:parentWindow];
                break;
            default:
                break;
        }
    }

    - (void)enableUserInterface {
        // ボタンを活性化
        [[self delegate] enableUserInterface];
    }

    - (void)resumeHIDCommand {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、ボタンを活性化し以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            [self enableUserInterface];
            return;
        }
        switch ([self command]) {
            case COMMAND_HID_BOOTLOADER_MODE:
                // ブートローダーモード遷移
                [self changeToBootloaderMode];
                break;
            default:
                break;
        }
    }

#pragma mark - For opening other window

    - (void)toolDFUWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ファームウェア更新画面を開く
        [[self delegate] disableUserInterface];
        [[self toolDFUCommand] toolDFUWindowWillOpen:sender parentWindow:parentWindow];
    }

    - (void)pivParamWindowWillOpenWithParent:(NSWindow *)parent {
        // PIV機能設定画面を表示
        [self doHIDCommand:COMMAND_OPEN_WINDOW_PIVPARAM sender:nil parentWindow:parent];
    }

    - (void)dfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ファームウェア更新処理を実行
        [[self toolUSBDFUCommand] dfuProcessWillStart:sender parentWindow:parentWindow toolHIDCommandRef:[self toolHIDCommand]];
    }

    - (void)dfuNewProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ファームウェア新規導入処理を実行するため、確認ダイアログを表示
        [[self delegate] disableUserInterface];
        [[self toolUSBDFUCommand] dfuNewProcessWillStart:sender parentWindow:parentWindow];
    }

    - (void)bleDfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ファームウェア更新処理を実行
        [[self toolBLEDFUCommand] bleDfuProcessWillStart:sender parentWindow:parentWindow];
    }

    - (void)pgpParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // OpenPGP機能設定画面を表示
        [self doHIDCommand:COMMAND_OPEN_WINDOW_PGPPARAM sender:sender parentWindow:parentWindow];
    }

#pragma mark - Perform bootloader mode

    - (void)changeToBootloaderMode {
        // ブートローダーモード遷移
        [self hidCommandStartedProcess:COMMAND_HID_BOOTLOADER_MODE];
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_HID_BOOTLOADER_MODE withData:nil forCommand:self];
    }

    - (void)didChangeToBootloaderMode:(Command)command response:(NSData *)response {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認し、画面に制御を戻す
        uint8_t *requestBytes = (uint8_t *)[response bytes];
        bool result = (requestBytes[0] == CTAP1_ERR_SUCCESS);
        [self commandDidProcess:command result:result message:MSG_BOOT_LOADER_MODE_UNSUPP];
    }

#pragma mark - Perform firmware reset

    - (void)doRequestForResetFirmware:(Command)command forCommandRef:(id)ref {
        // 認証器のファームウェアリセットを要求
        [[self toolHIDCommand] hidHelperWillProcess:command withData:nil forCommand:ref];
    }

    - (void)didResponseForResetFirmware:(Command)command response:(NSData *)response forCommandRef:(id)ref {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認し、エラーの場合は画面に制御を戻す
        uint8_t *requestBytes = (uint8_t *)[response bytes];
        if (requestBytes[0] != CTAP1_ERR_SUCCESS) {
            [self completedResetFirmware:command success:false forCommandRef:ref];
        } else {
            // 再接続まで待機 --> completedResetFirmware が呼び出される
            [[self toolHIDCommand] hidHelperWillDetectConnect:command forCommand:ref];
        }
    }

    - (void)completedResetFirmware:(Command)command success:(bool)success forCommandRef:(id)ref {
        // 画面に制御を戻す
        if ([ref isMemberOfClass:[ToolPIVCommand class]]) {
            [[self toolPIVCommand] commandDidResetFirmware:success];
        }
        if ([ref isMemberOfClass:[ToolPGPCommand class]]) {
            [[self toolPGPCommand] commandDidResetFirmware:success];
        }
    }

#pragma mark - Interface for AppDelegate

    - (void)commandStartedProcess:(Command)command type:(TransportType)type {
        // コマンド種別に対応する処理名称を設定
        [self setProcessNameOfCommand:nil];
        switch (command) {
            // BLE関連
            case COMMAND_BLE_DFU:
                [self setProcessNameOfCommand:PROCESS_NAME_BLE_DFU];
                break;
            // HID関連
            case COMMAND_HID_BOOTLOADER_MODE:
                [self setProcessNameOfCommand:PROCESS_NAME_BOOT_LOADER_MODE];
                break;
            case COMMAND_HID_FIRMWARE_RESET:
                [self setProcessNameOfCommand:PROCESS_NAME_FIRMWARE_RESET];
                break;
            case COMMAND_USB_DFU:
                [self setProcessNameOfCommand:PROCESS_NAME_USB_DFU];
                break;
            // BLE、HID共通
            default:
                break;
        }
        [[self delegate] commandStartedProcess:[self processNameOfCommand]];
    }

    - (void)commandDidProcess:(Command)command result:(bool)result message:(NSString *)message {
        [[self delegate] commandDidProcess:result message:message
                      processNameOfCommand:(command == COMMAND_NONE) ? nil : [self processNameOfCommand]];
        [self setProcessNameOfCommand:nil];
    }

#pragma mark - Call back from ToolHIDCommand

    - (void)hidCommandDidProcess:(Command)command toolCommandRef:(id)ref CMD:(uint8_t)cmd response:(NSData *)response {
        // 下位のコマンドクラスにデータと制御を引き渡す
        if ([ref isMemberOfClass:[ToolUSBDFUCommand class]]) {
            [[self toolUSBDFUCommand] hidCommandDidProcess:command CMD:cmd response:response];
        }
        if ([ref isMemberOfClass:[ToolAppCommand class]]) {
            // 画面に制御を戻す
            if (command == COMMAND_HID_BOOTLOADER_MODE) {
                [self didChangeToBootloaderMode:command response:response];
            }
        }
        if (command == COMMAND_HID_FIRMWARE_RESET) {
            [self didResponseForResetFirmware:command response:response forCommandRef:ref];
        }
    }

    - (void)hidCommandDidProcess:(Command)command result:(bool)result message:(NSString *)message {
        [self commandDidProcess:command result:result message:message];
    }

    - (void)hidCommandStartedProcess:(Command)command {
        [self commandStartedProcess:command type:TRANSPORT_HID];
    }

    - (void)hidCommandDidDetectConnect:(Command)command forCommandRef:(id)ref {
        [self notifyToolCommandMessage:MSG_HID_CONNECTED];
        [[ToolLogFile defaultLogger] info:MSG_HID_CONNECTED];
        // 所定のコマンドにHID接続を通知
        if (command == COMMAND_USB_DFU) {
            [[self toolUSBDFUCommand] hidCommandDidDetectConnect:command forCommandRef:ref];
        }
        if (command == COMMAND_HID_FIRMWARE_RESET) {
            [self completedResetFirmware:command success:true forCommandRef:ref];
        }
    }

    - (void)hidCommandDidDetectRemoval:(Command)command forCommandRef:(id)ref {
        [self notifyToolCommandMessage:MSG_HID_REMOVED];
        [[ToolLogFile defaultLogger] info:MSG_HID_REMOVED];
        // 所定のコマンドにHID接続切断を通知
        if (command == COMMAND_USB_DFU) {
            [[self toolUSBDFUCommand] hidCommandDidDetectRemoval:command forCommandRef:ref];
        }
    }

    - (void)notifyToolCommandMessage:(NSString *)message {
        [[self delegate] notifyAppCommandMessage:message];
    }

#pragma mark - Call back from ToolBLEDFUCommand

    - (void)notifyCommandStarted:(Command)command {
        [self commandStartedProcess:command type:TRANSPORT_BLE];
    }

    - (void)notifyCommandTerminated:(Command)command success:(bool)success message:(NSString *)message {
        [self commandDidProcess:command result:success message:message];
    }

    - (void)notifyMessage:(NSString *)message {
        [[self delegate] notifyAppCommandMessage:message];
    }

@end
