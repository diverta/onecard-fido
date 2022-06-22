//
//  AppHIDCommand.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/17.
//
#import "AppHIDCommand.h"
#import "FIDODefines.h"
#import "ToolHIDHelper.h"

@interface AppHIDCommand () <ToolHIDHelperDelegate>

    // ヘルパークラスの参照を保持
    @property (nonatomic) ToolHIDHelper *toolHIDHelper;
    // 実行コマンドを保持
    @property (nonatomic) Command        command;

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

    - (void)doRequestCommand:(Command)command {
        // 実行コマンドを保持
        [self setCommand:command];
        switch ([self command]) {
            case COMMAND_FIDO_ATTESTATION_INSTALL:
                // リクエスト実行に必要な新規CIDを取得するため、CTAPHID_INITを実行
                [self doRequestCtapHidInit];
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

    - (void)doRequestCtapHidInit {
        // CTAPHID_INITコマンドを実行
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
        switch ([self command]) {
            default:
                // 画面に制御を戻す
                [[self delegate] didResponseCommand:[self command] response:message success:false errorMessage:nil];
                break;
        }
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
            default:
                break;
        }
    }

    - (void)hidHelperDidResponseTimeout {
    }

@end
