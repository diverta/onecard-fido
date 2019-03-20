//
//  ToolHIDCommand.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2019/03/20.
//
#import <Foundation/Foundation.h>

#import "ToolCommonMessage.h"
#import "ToolHIDCommand.h"
#import "ToolHIDHelper.h"
#import "ToolPopupWindow.h"

@interface ToolHIDCommand () <ToolHIDHelperDelegate>

    @property (nonatomic) ToolHIDHelper *toolHIDHelper;
    @property (nonatomic) Command        command;

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

    - (void)doTestCtapHidInit {
        // CTAPHID_INITコマンドをテスト実行する
        NSData *message = [[NSData alloc] initWithBytes:nonceBytes length:sizeof(nonceBytes)];
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [[self toolHIDHelper] hidHelperWillSend:message CID:cid CMD:0x86];

    }

    - (void)doResponseCtapHidInit:(NSData *)message {
        // レスポンスメッセージの先頭８バイトと、リクエスト時のnonceが一致しているか確認
        char *requestBytes = (char *)[message bytes];
        bool result = (memcmp(requestBytes, nonceBytes, sizeof(nonceBytes)) == 0);

        // テキストエリアとポップアップの両方に表示させる処理終了メッセージを作成
        NSString *str = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE,
                         [ToolCommon processNameOfCommand:[self command]],
                         result? MSG_SUCCESS:MSG_FAILURE];
        // AppDelegateに制御を戻す
        [[self delegate] hidCommandDidProcess:[self command] result:result message:str];
    }

    - (void)hidHelperWillProcess:(Command)command {
        // USBポートに接続されていない場合は終了
        if (![[self toolHIDHelper] isDeviceConnected]) {
            [ToolPopupWindow critical:MSG_CMDTST_PROMPT_USB_PORT_SET informativeText:nil];
            [[self delegate] hidCommandDidProcess:command result:false message:nil];
            return;
        }
        // コマンドを待避
        [self setCommand:command];
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_TEST_CTAPHID_INIT:
                [self doTestCtapHidInit];
                break;
            default:
                // エラーメッセージを表示
                [ToolPopupWindow critical:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil];
                [[self delegate] hidCommandDidProcess:command result:false message:nil];
                break;
        }
    }

#pragma mark - Call back from ToolHIDHelper

    - (void)hidHelperDidReceive:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd {
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_TEST_CTAPHID_INIT:
                [self doResponseCtapHidInit:message];
                break;
            default:
                break;
        }
    }

@end
