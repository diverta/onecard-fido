//
//  AppHIDCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/07.
//
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "FIDODefines.h"
#import "ToolCommonMessage.h"
#import "ToolHIDHelper.h"

@interface AppHIDCommand () <ToolHIDHelperDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id          delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) ToolHIDHelper    *toolHIDHelper;
    // 実行コマンドを保持
    @property (nonatomic) Command           command;
    // CIDを保持
    @property (nonatomic) NSData           *cid;

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

#pragma mark - HID channel functions

    static char cidBytes[] = {0xff, 0xff, 0xff, 0xff};
    static char nonceBytes[] = {0x71, 0xcb, 0x1c, 0x3b, 0x10, 0x8e, 0xc9, 0x24};

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
        // HIDデバイスにリクエストを送信
        [self doRequestCommand:COMMAND_HID_CTAP2_INIT withCMD:HID_CMD_CTAPHID_INIT withData:message];
    }

    - (void)doResponseCtapHidInit:(NSData *)message {
        // レスポンスメッセージのnonceと、リクエスト時のnonceが一致していない場合は、画面に制御を戻す
        if ([self isWrongNonceBytes:message]) {
            [[self delegate] didResponseCommand:[self command] CMD:HID_CMD_CTAPHID_INIT response:message success:false errorMessage:MSG_HID_CMD_INIT_WRONG_NONCE];
            return;
        }
        // レスポンスからCIDを抽出し、内部で保持
        [self setCid:[self getNewCIDFrom:message]];
        // 上位クラスに制御を戻す
        [[self delegate] didResponseCommand:[self command] CMD:HID_CMD_CTAPHID_INIT response:message success:true errorMessage:nil];
    }

    - (void)doRequestCommand:(Command)command withCMD:(uint8_t)cmd withData:(NSData *)data {
        // 実行コマンドを保持
        [self setCommand:command];
        // HIDコマンド／データを送信（CIDはダミーを使用する）
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [[self toolHIDHelper] hidHelperWillSend:data CID:cid CMD:cmd];
    }

    - (void)doRequestCtap2Command:(Command)command withCMD:(uint8_t)cmd withData:(NSData *)data {
        // 実行コマンドを保持
        [self setCommand:command];
        // CTAPHID_INITから応答されたCIDを使用し、HIDコマンド／データを送信
        [[self toolHIDHelper] hidHelperWillSend:data CID:[self cid] CMD:cmd];
    }

#pragma mark - Call back from ToolHIDHelper

    - (void)hidHelperDidDetectConnect {
        [[self delegate] didDetectConnect];
    }

    - (void)hidHelperDidDetectRemoval {
        [[self delegate] didDetectRemoval];
    }

    - (void)hidHelperDidReceive:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd {
        // CTAPHID_INIT応答の場合
        if (cmd == HID_CMD_CTAPHID_INIT) {
            [self doResponseCtapHidInit:message];
            return;
        }
        // 正常終了扱い
        [[self delegate] didResponseCommand:[self command] CMD:cmd response:message success:true errorMessage:nil];
    }

    - (void)hidHelperDidResponseTimeout {
        // タイムアウト時はエラーメッセージを表示
        [[self delegate] didResponseCommand:[self command] CMD:HID_CMD_UNKNOWN_ERROR response:nil success:false errorMessage:MSG_HID_CMD_RESPONSE_TIMEOUT];
    }

@end
