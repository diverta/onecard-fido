//
//  FIDOPinSettingWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/19.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "FIDOPinSettingWindow.h"
#import "FIDOSettingCommand.h"
#import "ToolCommon.h"
#import "ToolCommonFunc.h"
#import "ToolPopupWindow.h"

@interface FIDOPinSettingWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // 画面項目の参照を保持
    @property (assign) IBOutlet NSSecureTextField          *fieldPin;
    @property (assign) IBOutlet NSSecureTextField          *fieldPinConfirm;
    @property (assign) IBOutlet NSSecureTextField          *fieldPinOld;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) id                          settingCommandRef;
    // PINコード設定処理のパラメーターを保持
    @property (nonatomic) FIDOSettingCommandParameter      *commandParameterRef;

@end

@implementation FIDOPinSettingWindow

    - (void)windowDidLoad {
        // 画面項目を初期化
        [super windowDidLoad];
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // 画面項目を初期値に設定
        [[self fieldPin]        setStringValue:@""];
        [[self fieldPinConfirm] setStringValue:@""];
        [[self fieldPinOld]     setStringValue:@""];

        // 最初の項目にフォーカス
        [[self fieldPin] becomeFirstResponder];
    }

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
        // コマンドクラスの参照を保持
        [self setSettingCommandRef:commandRef];
        // PINコード設定処理のパラメーターを保持
        [self setCommandParameterRef:parameterRef];
    }

    - (IBAction)buttonSetPinDidPress:(id)sender {
        // USBポートに接続されていない場合、処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 入力内容チェック
        if ([self checkEntries:false] == false) {
            return;
        }
        // PINコード、実行コマンドを設定して画面を閉じる
        [[self commandParameterRef] setPinNew:[[self fieldPin] stringValue]];
        [[self commandParameterRef] setPinOld:@""];
        [[self commandParameterRef] setCommand:COMMAND_CLIENT_PIN_SET];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonChangePinDidPress:(id)sender {
        // USBポートに接続されていない場合、処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 入力内容チェック
        if ([self checkEntries:true] == false) {
            return;
        }
        // PINコード、実行コマンドを設定して画面を閉じる
        [[self commandParameterRef] setPinNew:[[self fieldPin] stringValue]];
        [[self commandParameterRef] setPinOld:[[self fieldPinOld] stringValue]];
        [[self commandParameterRef] setCommand:COMMAND_CLIENT_PIN_CHANGE];
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // PINコード、実行コマンドを設定して画面を閉じる
        [[self commandParameterRef] setPinNew:@""];
        [[self commandParameterRef] setPinOld:@""];
        [[self commandParameterRef] setCommand:COMMAND_NONE];
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // 画面項目を初期化し、この画面を閉じる
        [self initFieldValue];
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合は処理中止
        FIDOSettingCommand *command = (FIDOSettingCommand *)[self settingCommandRef];
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:[self window] connected:[command isUSBHIDConnected]];
    }

#pragma mark - Check for entries

    - (bool) checkEntries:(bool)changePin {
        // 長さチェック
        if ([ToolCommon checkEntrySize:[self fieldPin]
                               minSize:PIN_CODE_SIZE_MIN maxSize:PIN_CODE_SIZE_MAX
                       informativeText:MSG_PROMPT_INPUT_NEW_PIN onWindow:[self window]] == false) {
            return false;
        }
        if ([ToolCommon checkEntrySize:[self fieldPinConfirm]
                               minSize:PIN_CODE_SIZE_MIN maxSize:PIN_CODE_SIZE_MAX
                       informativeText:MSG_PROMPT_INPUT_NEW_PIN_CONFIRM onWindow:[self window]] == false) {
            return false;
        }
        // 数字チェック
        if ([ToolCommon checkIsNumeric:[self fieldPin]
                       informativeText:MSG_PROMPT_INPUT_NEW_PIN_NUM onWindow:[self window]] == false) {
            return false;
        }
        if ([ToolCommon checkIsNumeric:[self fieldPinConfirm]
                       informativeText:MSG_PROMPT_INPUT_NEW_PIN_CONF_NUM onWindow:[self window]] == false) {
            return false;
        }
        // 変更ボタンがクリックされた場合は、変更前PINコードの入力チェックを実行
        if (changePin) {
            if ([ToolCommon checkEntrySize:[self fieldPinOld]
                                   minSize:PIN_CODE_SIZE_MIN maxSize:PIN_CODE_SIZE_MAX
                           informativeText:MSG_PROMPT_INPUT_OLD_PIN onWindow:[self window]] == false) {
                return false;
            }
            if ([ToolCommon checkIsNumeric:[self fieldPinOld]
                          informativeText:MSG_PROMPT_INPUT_OLD_PIN_NUM onWindow:[self window]] == false) {
                return false;
            }
        }
        // 確認用PINコードのチェック
        if ([ToolCommon compareEntry:[self fieldPinConfirm] srcField:[self fieldPin]
                     informativeText:MSG_PROMPT_INPUT_PIN_CONFIRM_CRCT onWindow:[self window]] == false) {
            return false;
        }
        return true;
    }

@end
