//
//  ToolAppCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/14.
//
#import "FIDODefines.h"
#import "ToolAppCommand.h"
#import "ToolBLECommand.h"
#import "ToolBLEDFUCommand.h"
#import "ToolCommonMessage.h"
#import "ToolContext.h"
#import "ToolDFUCommand.h"
#import "ToolHIDCommand.h"
#import "ToolLogFile.h"
#import "ToolPIVCommand.h"
#import "ToolPopupWindow.h"
#import "ToolPreferenceCommand.h"

@interface ToolAppCommand () <ToolHIDCommandDelegate, ToolBLECommandDelegate>

    @property (nonatomic) ToolBLECommand        *toolBLECommand;
    @property (nonatomic) ToolHIDCommand        *toolHIDCommand;
    @property (nonatomic) ToolBLEDFUCommand     *toolBLEDFUCommand;
    @property (nonatomic) ToolPreferenceCommand *toolPreferenceCommand;
    @property (nonatomic) ToolDFUCommand        *toolDFUCommand;
    @property (nonatomic) ToolPIVCommand        *toolPIVCommand;
    // 処理機能名称を保持
    @property (nonatomic) NSString *processNameOfCommand;
    // 実行するヘルスチェックの種別を保持
    @property (nonatomic) Command   healthCheckCommand;

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
            [self setToolBLECommand:[[ToolBLECommand alloc] initWithDelegate:self]];
            // 設定画面の初期設定
            [self setToolPreferenceCommand:[[ToolPreferenceCommand alloc] initWithDelegate:self toolHIDCommandRef:[self toolHIDCommand]]];
            // PIV機能の初期設定
            [self setToolPIVCommand:[[ToolPIVCommand alloc] initWithDelegate:self]];
            // DFU機能の初期設定
            [self setToolBLEDFUCommand:[[ToolBLEDFUCommand alloc] initWithDelegate:self]];
            [self setToolDFUCommand:[[ToolDFUCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

#pragma mark - Public methods

    - (void)doCommandPairing {
        // ペアリング実行
        [[self delegate] disableUserInterface];
        [[self toolBLECommand] bleCommandWillProcess:COMMAND_PAIRING];
    }

    - (void)doCommandEraseSkeyCert {
        if ([self checkForHIDCommand]) {
            if ([ToolPopupWindow promptYesNo:MSG_ERASE_SKEY_CERT
                             informativeText:MSG_PROMPT_ERASE_SKEY_CERT]) {
                // 鍵・証明書削除
                [[self delegate] disableUserInterface];
                [[self toolHIDCommand] hidHelperWillProcess:COMMAND_ERASE_SKEY_CERT];
            }
        }
    }

    - (void)doCommandInstallSkeyCert:(NSArray<NSString *> *)filePaths {
        if ([self checkForHIDCommand]) {
            if ([ToolPopupWindow promptYesNo:MSG_INSTALL_SKEY_CERT
                             informativeText:MSG_PROMPT_INSTL_SKEY_CERT]) {
                // 鍵・証明書インストール
                [[self delegate] disableUserInterface];
                [[self toolHIDCommand] setInstallParameter:COMMAND_INSTALL_SKEY_CERT
                                              skeyFilePath:filePaths[0]
                                              certFilePath:filePaths[1]];
                [[self toolHIDCommand] hidHelperWillProcess:COMMAND_INSTALL_SKEY_CERT];
            }
        }
    }

    - (void)doCommandTestCtapHidPing {
        if ([self checkForHIDCommand]) {
            // PINGテスト実行
            [[self delegate] disableUserInterface];
            [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TEST_CTAPHID_PING];
        }
    }

    - (void)doCommandHidGetFlashStat {
        if ([self checkForHIDCommand]) {
            // Flash ROM情報取得
            [[self delegate] disableUserInterface];
            [[self toolHIDCommand] hidHelperWillProcess:COMMAND_HID_GET_FLASH_STAT];
        }
    }

    - (void)doCommandHidGetVersionInfo {
        if ([self checkForHIDCommand]) {
            // バージョン情報取得
            [[self delegate] disableUserInterface];
            [[self toolHIDCommand] hidHelperWillProcess:COMMAND_HID_GET_VERSION_INFO];
        }
    }

    - (void)doCommandTestRegister {
        // BLE U2Fヘルスチェック実行
        [[self delegate] disableUserInterface];
        [[self toolBLECommand] bleCommandWillProcess:COMMAND_TEST_REGISTER];
    }

    - (void)doCommandTestBlePing {
        // BLE PINGテスト実行
        [[self delegate] disableUserInterface];
        [[self toolBLECommand] bleCommandWillProcess:COMMAND_TEST_BLE_PING];
    }

    - (void)doCommandHidCtap2HealthCheck {
        if ([self checkForHIDCommand]) {
            // HID CTAP2ヘルスチェック実行
            [[self delegate] disableUserInterface];
            [self performHealthCheckCommand:COMMAND_TEST_MAKE_CREDENTIAL];
        }
    }

    - (void)doCommandHidU2fHealthCheck {
        if ([self checkForHIDCommand]) {
            // HID U2Fヘルスチェック実行
            [[self delegate] disableUserInterface];
            [self performHealthCheckCommand:COMMAND_TEST_REGISTER];
        }
    }

    - (void)doCommandEraseBond {
        if ([self checkForHIDCommand]) {
            if ([ToolPopupWindow promptYesNo:MSG_ERASE_BONDS
                             informativeText:MSG_PROMPT_ERASE_BONDS]) {
                // ペアリング情報削除
                [[self delegate] disableUserInterface];
                [[self toolHIDCommand] hidHelperWillProcess:COMMAND_ERASE_BONDS];
            }
        }
    }

    - (void)doCommandBLMode {
        if ([self checkForHIDCommand]) {
            if ([ToolPopupWindow promptYesNo:MSG_BOOT_LOADER_MODE
                             informativeText:MSG_PROMPT_BOOT_LOADER_MODE]) {
                // ブートローダーモード遷移
                [[self delegate] disableUserInterface];
                [self changeToBootloaderMode];
            }
        }
    }

    - (bool)checkForHIDCommand {
        // USBポートに接続されていない場合はfalse
        return [[self toolHIDCommand] checkUSBHIDConnection];
    }

#pragma mark - For opening other window

    - (void)setPinParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        if ([self checkForHIDCommand]) {
            // PINコード設定画面を開く
            [[self delegate] disableUserInterface];
            [[self toolHIDCommand] setPinParamWindowWillOpen:sender parentWindow:parentWindow];
        }
    }

    - (void)pinCodeParamWindowWillOpenForHID:(id)sender parentWindow:(NSWindow *)parentWindow {
        // HID CTAP2ヘルスチェック処理を実行するため、PINコード入力画面を開く
        [[self toolHIDCommand] pinCodeParamWindowWillOpen:sender parentWindow:parentWindow];
    }

    - (void)pinCodeParamWindowWillOpenForBLE:(id)sender parentWindow:(NSWindow *)parentWindow {
        // BLE CTAP2ヘルスチェック処理を実行するため、PINコード入力画面を開く
        [[self delegate] disableUserInterface];
        [[self toolBLECommand] pinCodeParamWindowWillOpen:sender parentWindow:parentWindow];
    }

    - (void)PreferenceWindowWillOpenWithParent:(NSWindow *)parent {
        if ([self checkForHIDCommand]) {
            // PIV機能設定画面を表示
            [[self delegate] disableUserInterface];
            [[self toolPIVCommand] commandWillOpenPreferenceWindowWithParent:parent];
        }
    }

    - (void)toolPreferenceWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ツール設定画面を開く
        [[self delegate] disableUserInterface];
        [[self toolPreferenceCommand] toolPreferenceWindowWillOpen:sender parentWindow:parentWindow];
    }

    - (void)dfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow {
        if ([self checkForHIDCommand]) {
            // ファームウェア更新処理を実行するため、DFU開始画面を表示
            [[self delegate] disableUserInterface];
            [[self toolDFUCommand] dfuProcessWillStart:sender parentWindow:parentWindow toolHIDCommandRef:[self toolHIDCommand]];
        }
    }

    - (void)dfuNewProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ファームウェア新規導入処理を実行するため、確認ダイアログを表示
        [[self delegate] disableUserInterface];
        [[self toolDFUCommand] dfuNewProcessWillStart:sender parentWindow:parentWindow];
    }

    - (void)bleDfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ファームウェア更新処理を実行するため、DFU開始画面を表示
        [[self delegate] disableUserInterface];
        [[self toolBLEDFUCommand] bleDfuProcessWillStart:sender parentWindow:parentWindow toolBLECommandRef:[self toolBLECommand]];
    }

#pragma mark - Perform health check

    - (void)performHealthCheckCommand:(Command)command {
        // 事前にツール設定照会を実行
        [self setHealthCheckCommand:command];
        [[self toolPreferenceCommand] toolPreferenceInquiryWillProcess];
    }

    - (void)toolPreferenceInquiryDidProcess:(bool)result {
        // 処理失敗時は、BLE自動認証機能を無効化し、ヘルスチェック処理を実行
        if (result == false) {
            [[ToolContext instance] setBleScanAuthEnabled:false];
            [self resumeHealthCheckCommand];
            return;
        }
        // ツール設定情報を共有情報に保持させる
        [[ToolContext instance] setBleScanAuthEnabled:[[self toolPreferenceCommand] bleScanAuthEnabled]];
        if ([[ToolContext instance] bleScanAuthEnabled]) {
            // ツール設定でBLE自動認証機能が有効化されている場合は確認メッセージを表示
            if ([ToolPopupWindow promptYesNo:MSG_PROMPT_START_HCHK_BLE_AUTH
                             informativeText:MSG_COMMENT_START_HCHK_BLE_AUTH] == false) {
                // メッセージダイアログでNOをクリックした場合は終了
                [self commandDidProcess:COMMAND_NONE result:true message:nil];
                return;
            }
        }
        // ヘルスチェック処理を実行
        [self resumeHealthCheckCommand];
    }

    - (void)resumeHealthCheckCommand {
        switch ([self healthCheckCommand]) {
            case COMMAND_TEST_MAKE_CREDENTIAL:
                // HID CTAP2ヘルスチェック処理を実行（PINコード入力画面を開くため、いったんホーム画面に制御を戻す）
                [[self delegate] pinCodeParamWindowWillOpenForHID];
                break;
            case COMMAND_TEST_REGISTER:
                // HID U2Fヘルスチェック処理を実行
                [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TEST_REGISTER];
                break;
            default:
                break;
        }
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

#pragma mark - Interface for AppDelegate

    - (void)commandStartedProcess:(Command)command type:(TransportType)type {
        // コマンド種別に対応する処理名称を設定
        [self setProcessNameOfCommand:nil];
        switch (command) {
            // BLE関連
            case COMMAND_PAIRING:
                [self setProcessNameOfCommand:PROCESS_NAME_PAIRING];
                break;
            case COMMAND_TEST_BLE_PING:
                [self setProcessNameOfCommand:PROCESS_NAME_TEST_BLE_PING];
                break;
            case COMMAND_BLE_DFU:
                [self setProcessNameOfCommand:PROCESS_NAME_BLE_DFU];
                break;
            // HID関連
            case COMMAND_ERASE_BONDS:
                [self setProcessNameOfCommand:PROCESS_NAME_ERASE_BONDS];
                break;
            case COMMAND_ERASE_SKEY_CERT:
                [self setProcessNameOfCommand:PROCESS_NAME_ERASE_SKEY_CERT];
                break;
            case COMMAND_INSTALL_SKEY_CERT:
                [self setProcessNameOfCommand:PROCESS_NAME_INSTALL_SKEY_CERT];
                break;
            case COMMAND_TEST_CTAPHID_PING:
                [self setProcessNameOfCommand:PROCESS_NAME_TEST_CTAPHID_PING];
                break;
            case COMMAND_HID_GET_FLASH_STAT:
                [self setProcessNameOfCommand:PROCESS_NAME_GET_FLASH_STAT];
                break;
            case COMMAND_HID_GET_VERSION_INFO:
                [self setProcessNameOfCommand:PROCESS_NAME_GET_VERSION_INFO];
                break;
            case COMMAND_HID_BOOTLOADER_MODE:
                [self setProcessNameOfCommand:PROCESS_NAME_BOOT_LOADER_MODE];
                break;
            case COMMAND_CLIENT_PIN_SET:
                [self setProcessNameOfCommand:PROCESS_NAME_CLIENT_PIN_SET];
                break;
            case COMMAND_USB_DFU:
                [self setProcessNameOfCommand:PROCESS_NAME_USB_DFU];
                break;
            case COMMAND_CLIENT_PIN_CHANGE:
                [self setProcessNameOfCommand:PROCESS_NAME_CLIENT_PIN_CHANGE];
                break;
            case COMMAND_AUTH_RESET:
                [self setProcessNameOfCommand:PROCESS_NAME_AUTH_RESET];
                break;
            // BLE、HID共通
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                if (type == TRANSPORT_BLE) {
                    [self setProcessNameOfCommand:PROCESS_NAME_BLE_CTAP2_HEALTHCHECK];
                }
                if (type == TRANSPORT_HID) {
                    [self setProcessNameOfCommand:PROCESS_NAME_HID_CTAP2_HEALTHCHECK];
                }
                break;
            case COMMAND_TEST_REGISTER:
            case COMMAND_TEST_AUTH_CHECK:
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                if (type == TRANSPORT_BLE) {
                    [self setProcessNameOfCommand:PROCESS_NAME_BLE_U2F_HEALTHCHECK];
                }
                if (type == TRANSPORT_HID) {
                    [self setProcessNameOfCommand:PROCESS_NAME_HID_U2F_HEALTHCHECK];
                }
                break;
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


#pragma mark - Call back from ToolBLECommand

    - (void)bleCommandDidProcess:(Command)command toolCommandRef:(id)ref result:(bool)result response:(NSData *)response {
        // 下位のコマンドクラスにデータと制御を引き渡す
        if ([ref isMemberOfClass:[ToolBLEDFUCommand class]]) {
            ToolBLEDFUCommand *toolBLEDFUCommand = (ToolBLEDFUCommand *)ref;
            [toolBLEDFUCommand toolBLECommandDidProcess:command success:result response:response];
        }
    }

    - (void)bleCommandDidProcess:(Command)command
                          result:(bool)result message:(NSString *)message {
        [self commandDidProcess:command result:result message:message];
    }

    - (void)bleCommandStartedProcess:(Command)command {
        [self commandStartedProcess:command type:TRANSPORT_BLE];
    }

    - (void)notifyToolCommandMessage:(NSString *)message {
        [[self delegate] notifyAppCommandMessage:message];
    }

#pragma mark - Call back from ToolHIDCommand

    - (void)hidCommandDidProcess:(Command)command toolCommandRef:(id)ref CMD:(uint8_t)cmd response:(NSData *)response {
        // 下位のコマンドクラスにデータと制御を引き渡す
        if ([ref isMemberOfClass:[ToolPreferenceCommand class]]) {
            [[self toolPreferenceCommand] hidCommandDidProcess:command CMD:cmd response:response];
        }
        if ([ref isMemberOfClass:[ToolDFUCommand class]]) {
            [[self toolDFUCommand] hidCommandDidProcess:command CMD:cmd response:response];
        }
        if ([ref isMemberOfClass:[ToolAppCommand class]]) {
            // 画面に制御を戻す
            if (command == COMMAND_HID_BOOTLOADER_MODE) {
                [self didChangeToBootloaderMode:command response:response];
            }
        }
    }

    - (void)hidCommandDidProcess:(Command)command result:(bool)result message:(NSString *)message {
        [self commandDidProcess:command result:result message:message];
    }

    - (void)hidCommandStartedProcess:(Command)command {
        [self commandStartedProcess:command type:TRANSPORT_HID];
    }

    - (void)hidCommandDidDetectConnect {
        [self notifyToolCommandMessage:MSG_HID_CONNECTED];
        [[ToolLogFile defaultLogger] info:MSG_HID_CONNECTED];
        // DFU処理にHID接続開始を通知
        [[self toolDFUCommand] hidCommandDidDetectConnect:[self toolHIDCommand]];
    }

    - (void)hidCommandDidDetectRemoval {
        [self notifyToolCommandMessage:MSG_HID_REMOVED];
        [[ToolLogFile defaultLogger] info:MSG_HID_REMOVED];
        // DFU処理にHID接続切断を通知
        [[self toolDFUCommand] hidCommandDidDetectRemoval:[self toolHIDCommand]];
    }

@end
