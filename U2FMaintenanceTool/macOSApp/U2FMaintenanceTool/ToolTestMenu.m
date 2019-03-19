//
//  ToolTestMenu.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2019/03/19.
//
#import <Foundation/Foundation.h>

#import "ToolCommonMessage.h"
#import "ToolPopupWindow.h"
#import "ToolTestMenu.h"

@interface ToolTestMenu ()

    @property (nonatomic) ToolHIDHelper *toolHIDHelper;

@end

@implementation ToolTestMenu

    - (id)initWithHelper:(ToolHIDHelper *)helper {
        [self setToolHIDHelper:helper];
        return self;
    }

    - (void)doTestCtapHidInit {
        // USBポートに接続されていない場合は終了
        if (![[self toolHIDHelper] isDeviceConnected]) {
            [ToolPopupWindow informational:MSG_CMDTST_PROMPT_USB_PORT_SET informativeText:nil];
            return;
        }
        // CTAPHID_INITコマンドをテスト実行する
        char nonce[] = {0x71, 0xcb, 0x1c, 0x3b, 0x10, 0x8e, 0xc9, 0x24};
        char cidBytes[] = {0xff, 0xff, 0xff, 0xff};
        NSData *message = [[NSData alloc] initWithBytes:nonce length:sizeof(nonce)];
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [[self toolHIDHelper] hidHelperWillSend:message cid:cid command:0x86];
    }

    - (void)doTestBleDummy {
        // エラーメッセージを表示する
        [ToolPopupWindow informational:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil];
    }

@end
