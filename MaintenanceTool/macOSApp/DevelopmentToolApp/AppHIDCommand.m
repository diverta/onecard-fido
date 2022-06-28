//
//  AppHIDCommand.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/17.
//
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "FIDODefines.h"
#import "ToolCommonMessage.h"
#import "ToolHIDHelper.h"

@interface AppHIDCommand () <ToolHIDHelperDelegate>

    // ヘルパークラスの参照を保持
    @property (nonatomic) ToolHIDHelper *toolHIDHelper;
    // 実行コマンドを保持
    @property (nonatomic) Command        command;
    // CIDを保持
    @property (nonatomic) NSData        *cid;

@end

@implementation AppHIDCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
            [self setToolHIDHelper:[[ToolHIDHelper alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        return [[self toolHIDHelper] isDeviceConnected];
    }

    - (void)doRequestCommand:(Command)command withData:(NSData *)data {
        // 実行コマンドを保持
        [self setCommand:command];
        switch ([self command]) {
            case COMMAND_FIDO_ATTESTATION_INSTALL:
            case COMMAND_FIDO_ATTESTATION_RESET:
                [self doRequestCtapHidInit];
                break;
            case COMMAND_FIDO_ATTESTATION_INSTALL_REQUEST:
                [self doRequestFidoAttestationInstall:data];
                break;
            case COMMAND_FIDO_ATTESTATION_RESET_REQUEST:
                [self doRequestFidoAttestationReset];
                break;
            default:
                break;
        }
    }

#pragma mark - HID channel functions

    char cidBytes[] = {0xff, 0xff, 0xff, 0xff};
    char nonceBytes[] = {0x71, 0xcb, 0x1c, 0x3b, 0x10, 0x8e, 0xc9, 0x24};

    - (bool)isWrongNonceBytes:(NSData *)hidInitResponseMessage {
        // レスポンスメッセージのnonce（先頭8バイト）と、リクエスト時のnonceが一致しているか確認
        char *responseBytes = (char *)[hidInitResponseMessage bytes];
        return (memcmp(responseBytes, nonceBytes, sizeof(nonceBytes)) != 0);
    }

    - (NSData *)getNewCIDFrom:(NSData *)hidInitResponseMessage {
        // CTAPHID_INITレスポンスからCID（9〜12バイト目）を抽出
        NSData *newCID = [hidInitResponseMessage subdataWithRange:NSMakeRange(8, 4)];
        return newCID;
    }

    - (void)doRequestCtapHidInit {
        // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITコマンドを実行
        NSData *message = [[NSData alloc] initWithBytes:nonceBytes length:sizeof(nonceBytes)];
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        // HIDデバイスにリクエストを送信
        [[self toolHIDHelper] hidHelperWillSend:message CID:cid CMD:HID_CMD_CTAPHID_INIT];
    }

    - (void)doResponseCtapHidInit:(NSData *)message {
        // レスポンスメッセージのnonceと、リクエスト時のnonceが一致していない場合は、画面に制御を戻す
        if ([self isWrongNonceBytes:message]) {
            [[self delegate] didResponseCommand:[self command] response:message success:false errorMessage:nil];
            return;
        }
        // レスポンスからCIDを抽出し、内部で保持
        [self setCid:[self getNewCIDFrom:message]];
        // 画面に制御を戻す
        [[self delegate] didResponseCommand:[self command] response:message success:true errorMessage:nil];
    }

#pragma mark - FIDO Attestation functions

    - (void)doRequestFidoAttestationInstall:(NSData *)installData {
        // コマンド 0xC8 を実行
        [[self toolHIDHelper] hidHelperWillSend:installData CID:[self cid] CMD:HID_CMD_INSTALL_ATTESTATION];
    }

    - (void)doResponseFidoAttestationInstall:(NSData *)message {
        // 画面に制御を戻す
        [[self delegate] didResponseCommand:[self command] response:message success:true errorMessage:nil];
    }

    - (void)doRequestFidoAttestationReset {
        // コマンド 0xC9 を実行
        NSData *data = [[NSData alloc] init];
        [[self toolHIDHelper] hidHelperWillSend:data CID:[self cid] CMD:HID_CMD_RESET_ATTESTATION];
    }

    - (void)doResponseFidoAttestationReset:(NSData *)message {
        // 画面に制御を戻す
        [[self delegate] didResponseCommand:[self command] response:message success:true errorMessage:nil];
    }

#pragma mark - Call back from ToolHIDHelper

    - (void)hidHelperDidDetectConnect {
        [[self delegate] didDetectConnect];
    }

    - (void)hidHelperDidDetectRemoval {
        [[self delegate] didDetectRemoval];
    }

    - (void)hidHelperDidReceive:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd {
        // コマンドに応じ、以下の処理に分岐
        switch (cmd) {
            case HID_CMD_CTAPHID_INIT:
                [self doResponseCtapHidInit:message];
                break;
            case HID_CMD_INSTALL_ATTESTATION:
                [self doResponseFidoAttestationInstall:message];
                break;
            case HID_CMD_RESET_ATTESTATION:
                [self doResponseFidoAttestationReset:message];
                break;
            default:
                // メッセージを画面表示
                [[self delegate] didResponseCommand:[self command] response:nil success:false errorMessage:MSG_APP_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

    - (void)hidHelperDidResponseTimeout {
        // タイムアウト時はエラーメッセージを表示
        [[self delegate] didResponseCommand:[self command] response:nil success:false errorMessage:MSG_HID_CMD_RESPONSE_TIMEOUT];
    }

@end
