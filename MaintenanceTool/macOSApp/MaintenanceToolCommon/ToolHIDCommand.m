//
//  ToolHIDCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/03/20.
//
#import <Foundation/Foundation.h>

#import "AppCommonMessage.h"
#import "ToolAppCommand.h"
#import "ToolCommonMessage.h"
#import "ToolHIDCommand.h"
#import "ToolHIDHelper.h"
#import "ToolPopupWindow.h"
#import "FIDODefines.h"
#import "ToolLogFile.h"

@interface ToolHIDCommand () <ToolHIDHelperDelegate>

    @property (nonatomic) ToolHIDHelper        *toolHIDHelper;

    // コマンド、送受信データを保持
    @property (nonatomic) Command   command;
    @property (nonatomic) NSData   *processData;
    // HID接続／切断の検知時、所定コマンドへの通知の要否を保持
    @property(nonatomic) bool       needNotifyDetectConnect;
    @property(nonatomic) bool       needNotifyDetectRemoval;
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
        return self;
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
            case COMMAND_TOOL_PREF_PARAM:
            case COMMAND_TOOL_PREF_PARAM_INQUIRY:
                [self doRequestToolPreferenceParameter:[self getNewCIDFrom:message]];
                break;
            case COMMAND_HID_BOOTLOADER_MODE:
                [self doRequestHidBootloaderMode:[self getNewCIDFrom:message]];
                break;
            case COMMAND_HID_FIRMWARE_RESET:
                [self doRequestHidFirmwareReset:[self getNewCIDFrom:message]];
            default:
                // 画面に制御を戻す
                [self commandDidProcess:[self command] result:false message:nil];
                break;
        }
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

    - (void)hidHelperWillProcess:(Command)command withData:(NSData *)data forCommand:(id)commandRef {
        // HID接続／切断検知時、所定のコマンドに通知しないようにする
        [self setNeedNotifyDetectConnect:false];
        [self setNeedNotifyDetectRemoval:false];
        // 他のコマンドから、コマンドバイトとリクエストメッセージ本体を受取り、コマンドを実行
        [self setToolCommandRef:commandRef];
        [self setProcessData:data];
        // コマンドを待避
        [self setCommand:command];
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_HID_GET_VERSION_FOR_DFU:
                [self doHidGetVersionInfoRequest];
                break;
            case COMMAND_HID_BOOTLOADER_MODE:
                [self doHidBootloaderMode];
                break;
            case COMMAND_HID_FIRMWARE_RESET:
                [self doHidFirmwareReset];
                break;
            case COMMAND_TOOL_PREF_PARAM:
            case COMMAND_TOOL_PREF_PARAM_INQUIRY:
                [self doToolPreferenceParameter];
                break;
            default:
                // エラーメッセージを表示
                [self commandDidProcess:[self command] result:false message:MSG_CMDTST_MENU_NOT_SUPPORTED];
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
        [self setNeedNotifyDetectRemoval:false];
        // コマンドと呼出元の参照を待避
        [self setCommand:command];
        [self setToolCommandRef:commandRef];
    }

    - (void)hidHelperWillDetectRemoval:(Command)command forCommand:(id)commandRef {
        // HID接続の切断が検知されたら、所定のコマンドに通知
        [self setNeedNotifyDetectConnect:false];
        [self setNeedNotifyDetectRemoval:true];
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
            case HID_CMD_CTAPHID_INIT:
                [self doResponseCtapHidInit:message];
                break;
            case HID_CMD_BOOTLOADER_MODE:
                [self doResponseHidBootloaderMode:message CMD:cmd];
                break;
            case HID_CMD_FIRMWARE_RESET:
                [self doResponseHidFirmwareReset:message CMD:cmd];
                break;
            case HID_CMD_TOOL_PREF_PARAM:
                [self doResponseToolPreferenceParameter:message CID:cid CMD:cmd];
                break;
            case HID_CMD_UNKNOWN_ERROR:
                [self hidHelperDidReceiveUnknownError:message CID:cid CMD:cmd];
                break;
            case HID_CMD_GET_VERSION_INFO:
                [self doResponseHidGetVersionInfo:message];
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
        if ([self needNotifyDetectRemoval]) {
            // HID接続切断検知を所定のコマンドに通知する必要がある場合
            [self setNeedNotifyDetectRemoval:false];
            [[self delegate] hidCommandDidDetectRemoval:[self command] forCommandRef:[self toolCommandRef]];
        } else {
            // HID接続切断検知を所定のコマンドに通知する必要がない場合
            [[self delegate] hidCommandDidDetectRemoval:COMMAND_NONE forCommandRef:nil];
        }
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
        return [[self toolHIDHelper] isDeviceConnected];
    }

@end
