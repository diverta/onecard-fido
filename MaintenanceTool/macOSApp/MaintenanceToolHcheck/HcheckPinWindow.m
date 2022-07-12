//
//  HcheckPinWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/12.
//
#import "HcheckCommand.h"
#import "HcheckPinWindow.h"
#import "ToolCommonFunc.h"

@interface HcheckPinWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // 画面項目の参照を保持
    @property (assign) IBOutlet NSSecureTextField          *fieldPin;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) id                          hcheckCommandRef;

@end

@implementation HcheckPinWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
        // コマンドクラスの参照を保持
        [self setHcheckCommandRef:commandRef];
    }

    - (IBAction)buttonOKDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK];
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
        HcheckCommand *command = (HcheckCommand *)[self hcheckCommandRef];
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:[self window] connected:[command isUSBHIDConnected]];
    }

@end
