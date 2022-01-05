//
//  ToolHIDCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/03/20.
//
#import <Foundation/Foundation.h>

#import "ToolAppCommand.h"
#import "ToolCommonMessage.h"
#import "ToolHIDCommand.h"
#import "ToolHIDHelper.h"
#import "ToolInstallCommand.h"
#import "ToolClientPINCommand.h"
#import "ToolCTAP2HealthCheckCommand.h"
#import "ToolU2FHealthCheckCommand.h"
#import "ToolPopupWindow.h"
#import "FIDODefines.h"
#import "ToolLogFile.h"

@interface ToolHIDCommand () <ToolHIDHelperDelegate>

    @property (nonatomic) ToolHIDHelper        *toolHIDHelper;
    @property (nonatomic) ToolInstallCommand   *toolInstallCommand;
    @property (nonatomic) ToolClientPINCommand *toolClientPINCommand;
    @property (nonatomic) ToolCTAP2HealthCheckCommand
                                               *toolCTAP2HealthCheckCommand;
    @property (nonatomic) ToolU2FHealthCheckCommand
                                               *toolU2FHealthCheckCommand;

    // コマンド、送受信データを保持
    @property (nonatomic) Command   command;
    @property (nonatomic) NSData   *processData;
    @property (nonatomic) NSString *skeyFilePath;
    @property (nonatomic) NSString *certFilePath;

    // 送信PINGデータを保持
    @property(nonatomic) NSData    *pingData;
    // HID接続の検知時、所定コマンドへの通知の要否を保持
    @property(nonatomic) bool       needNotifyDetectConnect;
    // 呼び出し元のコマンドオブジェクト参照を保持
    @property(nonatomic, weak) id   toolCommandRef;

@end

@implementation ToolHIDCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolHIDCommandDelegate>)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
        }
        [self setToolHIDHelper:[[ToolHIDHelper alloc] initWithDelegate:self]];
        [self setToolInstallCommand:[[ToolInstallCommand alloc] init]];
        [self setToolClientPINCommand:[[ToolClientPINCommand alloc] init]];
        [self setToolCTAP2HealthCheckCommand:[[ToolCTAP2HealthCheckCommand alloc] init]];
        [[self toolCTAP2HealthCheckCommand] setTransportParam:TRANSPORT_HID
                                               toolBLECommand:nil
                                               toolHIDCommand:self];
        [self setToolU2FHealthCheckCommand:[[ToolU2FHealthCheckCommand alloc] init]];
        [[self toolU2FHealthCheckCommand] setTransportParam:TRANSPORT_HID
                                             toolBLECommand:nil
                                             toolHIDCommand:self];
        return self;
    }

#pragma mark - Parameter set

    - (void)setInstallParameter:(Command)command
                   skeyFilePath:(NSString *)skeyFilePath certFilePath:(NSString *)certFilePath {
        // インストール対象の鍵・証明書ファイルパスを保持
        [self setSkeyFilePath:skeyFilePath];
        [self setCertFilePath:certFilePath];
    }

#pragma mark - Constants for test

    char cidBytes[] = {0xff, 0xff, 0xff, 0xff};
    char nonceBytes[] = {0x71, 0xcb, 0x1c, 0x3b, 0x10, 0x8e, 0xc9, 0x24};

#pragma mark - Command functions

    - (void)doRequest:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd {
        // HIDデバイスにリクエストを送信
        [[self toolHIDHelper] hidHelperWillSend:message CID:cid CMD:cmd];
    }

    - (void)doRequestCtapHidInit {
        // CTAPHID_INITコマンドを実行
        NSData *message = [[NSData alloc] initWithBytes:nonceBytes length:sizeof(nonceBytes)];
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [self doRequest:message CID:cid CMD:HID_CMD_CTAPHID_INIT];
    }

    - (void)doResponseCtapHidInit:(NSData *)message {
        if ([self isCorrectNonceBytes:message] == false) {
            // レスポンスメッセージのnonceと、リクエスト時のnonceが一致していない場合は、
            // 画面に制御を戻す
            [self commandDidProcess:[self command] result:false message:nil];
        }
        switch ([self command]) {
            case COMMAND_TEST_CTAPHID_PING:
                // 受領したCIDを使用し、CTAPHID_PINGコマンドを実行
                [self doRequestCtapHidPing:[self getNewCIDFrom:message]];
                break;
            case COMMAND_CLIENT_PIN_SET:
            case COMMAND_CLIENT_PIN_CHANGE:
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
            case COMMAND_AUTH_RESET:
            case COMMAND_INSTALL_SKEY_CERT:
                // 受領したCIDを使用し、GetKeyAgreement／authenticatorResetコマンドを実行
                [[self toolCTAP2HealthCheckCommand] setCID:[self getNewCIDFrom:message]];
                [[self toolCTAP2HealthCheckCommand] doCTAP2Request:[self command]];
                break;
            case COMMAND_TEST_REGISTER:
            case COMMAND_TEST_AUTH_CHECK:
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                // 受領したCIDを使用し、U2FRegister／Authenticateコマンドを実行
                [[self toolU2FHealthCheckCommand] setCID:[self getNewCIDFrom:message]];
                [[self toolU2FHealthCheckCommand] doU2FRequest:[self command]];
                break;
            case COMMAND_TOOL_PREF_PARAM:
            case COMMAND_TOOL_PREF_PARAM_INQUIRY:
                [self doRequestToolPreferenceParameter:[self getNewCIDFrom:message]];
                break;
            case COMMAND_HID_BOOTLOADER_MODE:
                [self doRequestHidBootloaderMode:[self getNewCIDFrom:message]];
                break;
            case COMMAND_HID_FIRMWARE_RESET:
                [self doRequestHidFirmwareReset:[self getNewCIDFrom:message]];
                break;
            case COMMAND_ERASE_SKEY_CERT:
                [self doRequestEraseSkeyCert:[self getNewCIDFrom:message]];
                break;
            case COMMAND_ERASE_BONDS:
                [self doRequestEraseBonds:[self getNewCIDFrom:message]];
                break;
            default:
                // 画面に制御を戻す
                [self commandDidProcess:[self command] result:false message:nil];
                break;
        }
    }

    - (void)doTestCtapHidPing {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doRequestCtapHidPing:(NSData *)cid {
        // 100バイトのランダムなPINGデータを生成
        [self setPingData:[ToolCommon generateRandomBytesDataOf:100]];
        // メッセージを編集し、CTAPHID_PINGコマンドを実行
        [self doRequest:[self pingData] CID:cid CMD:HID_CMD_CTAPHID_PING];
    }

    - (void)doResponseCtapHidPing:(NSData *)message {
        // PINGレスポンスの内容をチェックし、画面に制御を戻す
        bool result = [message isEqualToData:[self pingData]];
        [self commandDidProcess:[self command] result:result message:MSG_CMDTST_INVALID_PING];
    }

    - (void)doHidGetFlashStat {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        // コマンド 0xC2 を実行（メッセージはブランクとする）
        NSData *message = [[NSData alloc] init];
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [self doRequest:message CID:cid CMD:HID_CMD_GET_FLASH_STAT];
    }

    - (void)doResponseHidGetFlashStat:(NSData *)message {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *requestBytes = (uint8_t *)[message bytes];
        if (requestBytes[0] != CTAP1_ERR_SUCCESS) {
            // エラーの場合は画面に制御を戻す
            [self commandDidProcess:[self command] result:false message:nil];
            return;
        }
        // 戻りメッセージから、取得情報CSVを抽出
        NSData *responseBytes = [ToolCommon extractCBORBytesFrom:message];
        NSString *responseCSV = [[NSString alloc] initWithData:responseBytes encoding:NSASCIIStringEncoding];
        [[ToolLogFile defaultLogger] debugWithFormat:@"Flash ROM statistics: %@", responseCSV];
        // 情報取得CSVから空き領域に関する情報を抽出
        NSString *strUsed = @"";
        NSString *strAvail = @"";
        NSString *strCorrupt = @"";
        for (NSString *element in [responseCSV componentsSeparatedByString:@","]) {
            NSArray *items = [element componentsSeparatedByString:@"="];
            NSString *key = [items objectAtIndex:0];
            NSString *val = [items objectAtIndex:1];
            if ([key isEqualToString:@"words_used"]) {
                strUsed = val;
            } else if ([key isEqualToString:@"words_available"]) {
                strAvail = val;
            } else if ([key isEqualToString:@"corruption"]) {
                strCorrupt = val;
            }
        }
        // 空き容量、破損状況を画面に表示
        NSString *rateText;
        if ([strUsed length] > 0 && [strAvail length] > 0) {
            float avail = [strAvail floatValue];
            float remaining = avail - [strUsed floatValue];
            float rate = remaining / avail * 100.0;
            rateText = [NSString stringWithFormat:MSG_FSTAT_REMAINING_RATE, rate];
        } else {
            rateText = MSG_FSTAT_NON_REMAINING_RATE;
        }
        NSString *corruptText = [strCorrupt isEqualToString:@"0"] ?
            MSG_FSTAT_CORRUPTING_AREA_NOT_EXIST : MSG_FSTAT_CORRUPTING_AREA_EXIST;
        // 画面に制御を戻す
        [self displayMessage:[NSString stringWithFormat:@"  %1$@%2$@", rateText, corruptText]];
        [self commandDidProcess:[self command] result:true message:nil];
    }

    - (void)doHidGetVersionInfo {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        // バージョン照会リクエストを実行
        [self doHidGetVersionInfoRequest];
    }

    - (void)doHidGetVersionInfoRequest {
        // コマンド 0xC3 を実行（メッセージはブランクとする）
        NSData *message = [[NSData alloc] init];
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [self doRequest:message CID:cid CMD:HID_CMD_GET_VERSION_INFO];
    }

    - (void)doResponseHidGetVersionInfo:(NSData *)message {
        // 別クラスからの呼び出しの場合、上位コマンドクラスに制御を戻す
        if ([self toolCommandRef]) {
            [[self delegate] hidCommandDidProcess:[self command] toolCommandRef:[self toolCommandRef] CMD:0x00 response:message];
            return;
        }
        // 戻りメッセージから、取得情報CSVを抽出
        NSData *responseBytes = [ToolCommon extractCBORBytesFrom:message];
        NSString *responseCSV = [[NSString alloc] initWithData:responseBytes encoding:NSASCIIStringEncoding];
        // 情報取得CSVからバージョン情報を抽出
        NSArray<NSString *> *array = [ToolCommon extractValuesFromVersionInfo:responseCSV];
        NSString *strDeviceName = array[0];
        NSString *strFWRev = array[1];
        NSString *strHWRev = array[2];
        NSString *strSecic = array[3];
        // 画面に制御を戻す
        [self displayMessage:MSG_VERSION_INFO_HEADER];
        [self displayMessage:[NSString stringWithFormat:MSG_VERSION_INFO_DEVICE_NAME, strDeviceName]];
        [self displayMessage:[NSString stringWithFormat:MSG_VERSION_INFO_FW_REV, strFWRev]];
        [self displayMessage:[NSString stringWithFormat:MSG_VERSION_INFO_HW_REV, strHWRev]];
        // セキュアICの搭載有無を表示
        if ([strSecic length] > 0) {
            [self displayMessage:MSG_VERSION_INFO_SECURE_IC_AVAIL];
        } else {
            [self displayMessage:MSG_VERSION_INFO_SECURE_IC_UNAVAIL];
        }
        [self commandDidProcess:[self command] result:true message:nil];
    }

    - (void)doHidBootloaderMode {
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doRequestHidBootloaderMode:(NSData *)cid {
        // コマンド 0xC5 を実行（メッセージはブランクとする）
        NSData *message = [[NSData alloc] init];
        [self doRequest:message CID:cid CMD:HID_CMD_BOOTLOADER_MODE];
    }

    - (void)doResponseHidBootloaderMode:(NSData *)message CMD:(uint8_t)cmd {
        // 別クラスからの呼び出しの場合、上位コマンドクラスに制御を戻す
        if ([self toolCommandRef]) {
            [[self delegate] hidCommandDidProcess:[self command] toolCommandRef:[self toolCommandRef] CMD:cmd response:message];
            return;
        }
    }

    - (void)doHidFirmwareReset {
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doRequestHidFirmwareReset:(NSData *)cid {
        // コマンド 0xC7 を実行（メッセージはブランクとする）
        NSData *message = [[NSData alloc] init];
        [self doRequest:message CID:cid CMD:HID_CMD_FIRMWARE_RESET];
    }

    - (void)doResponseHidFirmwareReset:(NSData *)message CMD:(uint8_t)cmd {
        // 別クラスからの呼び出しの場合、上位コマンドクラスに制御を戻す
        if ([self toolCommandRef]) {
            [[self delegate] hidCommandDidProcess:[self command] toolCommandRef:[self toolCommandRef] CMD:cmd response:message];
            return;
        }
    }

    - (void)doEraseBonds {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doRequestEraseBonds:(NSData *)cid {
        // メッセージを編集し、コマンド 0xC6 を実行
        NSData *message = [[NSData alloc] init];
        [self doRequest:message CID:cid CMD:HID_CMD_ERASE_BONDS];
    }

    - (void)doEraseSkeyCert {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doRequestEraseSkeyCert:(NSData *)cid {
        // メッセージを編集し、コマンド 0xC0 を実行
        NSData *message = [[self toolInstallCommand] generateEraseSkeyCertMessage:[self command]];
        [self doRequest:message CID:cid CMD:HID_CMD_ERASE_SKEY_CERT];
    }

    - (void)doInstallSkeyCert {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doRequestInstallSkeyCert:(NSData *)messageKeyAgreement CID:(NSData *)cid {
        // 公開鍵を抽出
        if ([[self toolInstallCommand] extractKeyAgreement:messageKeyAgreement] == false) {
            // 処理が失敗した場合は、AppDelegateに制御を戻す
            [self commandDidProcess:[self command] result:false message:[[self toolInstallCommand] lastErrorMessage]];
            return;
        }
        // 鍵ファイル・証明書ファイルから、バイナリーデータを読込んで１本にマージ
        NSData *skeyCertBinaryData = [[self toolInstallCommand]
                                      extractSkeyCertBinaryData:[self command]
                                      skeyFilePath:[self skeyFilePath] certFilePath:[self certFilePath]];
        if (skeyCertBinaryData == nil) {
            // 処理が失敗した場合は、AppDelegateに制御を戻す
            [self commandDidProcess:[self command] result:false message:[[self toolInstallCommand] lastErrorMessage]];
            return;
        }
        // 共通鍵により鍵・証明書を暗号化し、コマンド実行のためのCBORメッセージを生成
        NSData *skeyCertInstallCbor =
            [[self toolInstallCommand] generateSkeyCertInstallCbor:skeyCertBinaryData];
        if (skeyCertInstallCbor == nil) {
            // 処理が失敗した場合は、AppDelegateに制御を戻す
            [self commandDidProcess:[self command] result:false message:[[self toolInstallCommand] lastErrorMessage]];
            return;
        }

        // コマンド 0xC1 を実行
        [self doRequest:skeyCertInstallCbor CID:cid CMD:HID_CMD_INSTALL_SKEY_CERT];
    }

    - (void)doResponseMaintenanceCommand:(NSData *)message {
        // ステータスコードを確認し、画面に制御を戻す
        [self commandDidProcess:[self command] result:[[self toolCTAP2HealthCheckCommand] checkStatusCode:message] message:nil];
    }

    - (void)doClientPin {
        // コマンド開始メッセージを画面表示
        [self displayStartMessage];
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doClientPinSetOrChange:(NSData *)message CID:(NSData *)cid {
        // メッセージを編集し、GetKeyAgreementサブコマンドを実行
        NSData *request = [[self toolClientPINCommand] generateClientPinSetRequestWith:message];
        if (request == nil) {
            [self commandDidProcess:[self command] result:false message:nil];
            return;
        }
        // コマンドを実行
        [self doRequest:request CID:cid CMD:HID_CMD_CTAPHID_CBOR];
    }

    - (void)doCtap2HealthCheck {
        // コマンド開始メッセージを画面表示
        if ([self command] == COMMAND_TEST_MAKE_CREDENTIAL) {
            [self displayStartMessage];
        }
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doResponseCtapHidCbor:(NSData *)message
                            CID:(NSData *)cid CMD:(uint8_t)cmd {
        // ステータスコードを確認し、NGの場合は画面に制御を戻す
        [[self toolCTAP2HealthCheckCommand] setCID:cid];
        [[self toolCTAP2HealthCheckCommand] doCTAP2Response:[self command] responseMessage:message];
    }

    - (void)doU2FHealthCheck {
        // コマンド開始メッセージを画面表示
        if ([self command] == COMMAND_TEST_REGISTER) {
            [self displayStartMessage];
        }
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doResponseU2FHidMsg:(NSData *)message
                            CID:(NSData *)cid CMD:(uint8_t)cmd {
        // ステータスコードを確認し、NGの場合は画面に制御を戻す
        [[self toolU2FHealthCheckCommand] setCID:cid];
        [[self toolU2FHealthCheckCommand] doU2FResponse:[self command] responseMessage:message];
    }

    - (void)hidHelperWillProcess:(Command)command withData:(NSData *)data forCommand:(id)commandRef {
        // HID接続検知時、所定のコマンドに通知しないようにする
        [self setNeedNotifyDetectConnect:false];
        // 他のコマンドから、コマンドバイトとリクエストメッセージ本体を受取り、コマンドを実行
        [self setToolCommandRef:commandRef];
        [self setProcessData:data];
        // コマンドを待避
        [self setCommand:command];
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_TEST_CTAPHID_PING:
                [self doTestCtapHidPing];
                break;
            case COMMAND_HID_GET_FLASH_STAT:
                [self doHidGetFlashStat];
                break;
            case COMMAND_HID_GET_VERSION_INFO:
                [self doHidGetVersionInfo];
                break;
            case COMMAND_HID_GET_VERSION_FOR_DFU:
                [self doHidGetVersionInfoRequest];
                break;
            case COMMAND_HID_BOOTLOADER_MODE:
                [self doHidBootloaderMode];
                break;
            case COMMAND_HID_FIRMWARE_RESET:
                [self doHidFirmwareReset];
                break;
            case COMMAND_ERASE_BONDS:
                [self doEraseBonds];
                break;
            case COMMAND_ERASE_SKEY_CERT:
                [self doEraseSkeyCert];
                break;
            case COMMAND_INSTALL_SKEY_CERT:
                [self doInstallSkeyCert];
                break;
            case COMMAND_CLIENT_PIN_SET:
            case COMMAND_CLIENT_PIN_CHANGE:
            case COMMAND_AUTH_RESET:
                [self doClientPin];
                break;
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                [self doCtap2HealthCheck];
                break;
            case COMMAND_TEST_REGISTER:
            case COMMAND_TEST_AUTH_CHECK:
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                [self doU2FHealthCheck];
                break;
            case COMMAND_TOOL_PREF_PARAM:
            case COMMAND_TOOL_PREF_PARAM_INQUIRY:
                [self doToolPreferenceParameter];
                break;
            default:
                // エラーメッセージを表示
                [ToolPopupWindow critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil];
                [self commandDidProcess:[self command] result:false message:nil];
                break;
        }
    }

#pragma mark - For process from other command

    - (void)hidHelperWillProcess:(Command)command {
        [self hidHelperWillProcess:command withData:nil forCommand:nil];
    }

    - (void)hidHelperWillDetectConnect:(Command)command forCommand:(id)commandRef {
        // HID接続が検知されたら、所定のコマンドに通知
        [self setNeedNotifyDetectConnect:true];
        // コマンドと呼出元の参照を待避
        [self setCommand:command];
        [self setToolCommandRef:commandRef];
    }

#pragma mark - For tool preference parameters

    - (void)doToolPreferenceParameter {
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
        [self doRequestCtapHidInit];
    }

    - (void)doRequestToolPreferenceParameter:(NSData *)cid {
        // メッセージを編集し、コマンドを実行
        [self doRequest:[self processData] CID:cid CMD:HID_CMD_TOOL_PREF_PARAM];
    }

    - (void)doResponseToolPreferenceParameter:(NSData *)message
                            CID:(NSData *)cid CMD:(uint8_t)cmd {
        // 上位コマンドクラスに制御を戻し、コマンドバイトと応答メッセージ本体を戻す
        [[self delegate] hidCommandDidProcess:[self command] toolCommandRef:[self toolCommandRef] CMD:cmd response:message];
    }

#pragma mark - Call back from ToolHIDHelper

    - (void)hidHelperDidReceive:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd {
        // コマンドに応じ、以下の処理に分岐
        switch (cmd) {
            case HID_CMD_CTAPHID_PING:
                [self doResponseCtapHidPing:message];
                break;
            case HID_CMD_CTAPHID_INIT:
                [self doResponseCtapHidInit:message];
                break;
            case HID_CMD_GET_FLASH_STAT:
                [self doResponseHidGetFlashStat:message];
                break;
            case HID_CMD_GET_VERSION_INFO:
                [self doResponseHidGetVersionInfo:message];
                break;
            case HID_CMD_BOOTLOADER_MODE:
                [self doResponseHidBootloaderMode:message CMD:cmd];
                break;
            case HID_CMD_FIRMWARE_RESET:
                [self doResponseHidFirmwareReset:message CMD:cmd];
                break;
            case HID_CMD_ERASE_BONDS:
            case HID_CMD_ERASE_SKEY_CERT:
            case HID_CMD_INSTALL_SKEY_CERT:
                [self doResponseMaintenanceCommand:message];
                break;
            case HID_CMD_CTAPHID_CBOR:
                [self doResponseCtapHidCbor:message CID:cid CMD:cmd];
                break;
            case HID_CMD_MSG:
                [self doResponseU2FHidMsg:message CID:cid CMD:cmd];
                break;
            case HID_CMD_TOOL_PREF_PARAM:
                [self doResponseToolPreferenceParameter:message CID:cid CMD:cmd];
                break;
            case HID_CMD_UNKNOWN_ERROR:
                [self hidHelperDidReceiveUnknownError:message CID:cid CMD:cmd];
                break;
            default:
                break;
        }
    }

    - (void)hidHelperDidReceiveUnknownError:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd {
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_HID_BOOTLOADER_MODE:
                // DFU処理クラスに制御を戻す
                [self doResponseHidBootloaderMode:message CMD:cmd];
                break;
            case COMMAND_HID_FIRMWARE_RESET:
                // DFU処理クラスに制御を戻す
                [self doResponseHidFirmwareReset:message CMD:cmd];
                break;
            default:
                // メッセージを画面表示
                [[self delegate] hidCommandDidProcess:[self command] result:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

    - (void)hidHelperDidResponseTimeout {
        // タイムアウト時はエラーメッセージを表示
        [self commandDidProcess:[self command] result:false message:MSG_HID_CMD_RESPONSE_TIMEOUT];
    }

    - (void)hidHelperDidDetectConnect {
        if ([self needNotifyDetectConnect]) {
            // HID接続検知を所定のコマンドに通知する必要がある場合
            [self setNeedNotifyDetectConnect:false];
            [[self delegate] hidCommandDidDetectConnect:[self command] forCommandRef:[self toolCommandRef]];
        } else {
            // HID接続検知を所定のコマンドに通知する必要がない場合
            [[self delegate] hidCommandDidDetectConnect:COMMAND_NONE forCommandRef:nil];
        }
    }

    - (void)hidHelperDidDetectRemoval {
        [[self delegate] hidCommandDidDetectRemoval];
    }

#pragma mark - Common method

    - (void)displayMessage:(NSString *)string {
        // メッセージを画面表示
        [[self delegate] notifyToolCommandMessage:string];
    }

    - (void)displayStartMessage {
        // 指定コマンド種別の処理開始を通知
        [[self delegate] hidCommandStartedProcess:[self command]];
    }

    - (void)commandDidProcess:(Command)command result:(bool)result message:(NSString *)message {
        // 即時でアプリケーションに制御を戻す
        [[self delegate] hidCommandDidProcess:command result:result message:message];
    }

    - (NSData *)getNewCIDFrom:(NSData *)hidInitResponseMessage {
        // CTAPHID_INITレスポンスからCID（9〜12バイト目）を抽出
        NSData *newCID = [hidInitResponseMessage subdataWithRange:NSMakeRange(8, 4)];
        return newCID;
    }

    - (bool)isCorrectNonceBytes:(NSData *)hidInitResponseMessage {
        // レスポンスメッセージのnonce（先頭8バイト）と、リクエスト時のnonceが一致しているか確認
        char *responseBytes = (char *)[hidInitResponseMessage bytes];
        return (memcmp(responseBytes, nonceBytes, sizeof(nonceBytes)) == 0);
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        if (![[self toolHIDHelper] isDeviceConnected]) {
            [ToolPopupWindow critical:MSG_CMDTST_PROMPT_USB_PORT_SET informativeText:nil];
            return false;
        }
        return true;
    }

#pragma mark - Interface for SetPinParamWindow

    - (void)setPinParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ダイアログをモーダルで表示
        [[self toolClientPINCommand] setPinParamWindowWillOpen:sender
                                                  parentWindow:parentWindow toolCommand:self];
    }

    - (void)setPinParamWindowDidClose {
        // AppDelegateに制御を戻す（ポップアップメッセージは表示しない）
        [self commandDidProcess:COMMAND_NONE result:true message:nil];
    }

#pragma mark - Interface for PinCodeParamWindow

    - (void)pinCodeParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ダイアログをモーダルで表示
        [[self toolCTAP2HealthCheckCommand] pinCodeParamWindowWillOpen:sender
                                            parentWindow:parentWindow];
    }

    - (void)pinCodeParamWindowDidClose {
        // AppDelegateに制御を戻す（ポップアップメッセージは表示しない）
        [self commandDidProcess:COMMAND_NONE result:true message:nil];
    }

@end
