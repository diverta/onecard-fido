//
//  HcheckPinWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/12.
//
#import "AppCommonMessage.h"
#import "HcheckCommand.h"
#import "HcheckPinWindow.h"
#import "ToolCommon.h"
#import "ToolCommonFunc.h"

@interface HcheckPinWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // 画面項目の参照を保持
    @property (assign) IBOutlet NSSecureTextField          *fieldPin;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) id                          hcheckCommandRef;
    // ヘルスチェック処理のパラメーターを保持
    @property (nonatomic) HcheckCommandParameter           *commandParameterRef;

@end

@implementation HcheckPinWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
        // コマンドクラスの参照を保持
        [self setHcheckCommandRef:commandRef];
        // ヘルスチェック処理のパラメーターを保持
        [self setCommandParameterRef:parameterRef];
    }

    - (IBAction)buttonOKDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 入力内容チェック
        if ([self checkEntries:sender] == false) {
            return;
        }
        // このウィンドウを終了
        [[self commandParameterRef] setPin:[[self fieldPin] stringValue]];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // このウィンドウを終了
        [[self commandParameterRef] setPin:@""];
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self fieldPin] setStringValue:@""];
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合は処理中止
        HcheckCommand *command = (HcheckCommand *)[self hcheckCommandRef];
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:[self window] connected:[command isUSBHIDConnected]];
    }

#pragma mark - Check for entries and process

    - (bool) checkEntries:(id)sender {
        // 長さチェック
        if ([ToolCommon checkEntrySize:[self fieldPin] minSize:PIN_CODE_SIZE_MIN maxSize:PIN_CODE_SIZE_MAX
                       informativeText:MSG_PROMPT_INPUT_CUR_PIN onWindow:[self window]] == false) {
            return false;
        }
        // 数字チェック
        if ([ToolCommon checkIsNumeric:[self fieldPin] informativeText:MSG_PROMPT_INPUT_CUR_PIN_NUM onWindow:[self window]] == false) {
            return false;
        }
        return true;
    }

@end
