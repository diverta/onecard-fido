//
//  SetPinParamWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/02/27.
//
#import "SetPinParamWindow.h"
#import "ToolCommon.h"
#import "ToolCommonMessage.h"

@interface SetPinParamWindow ()

    @property (assign) IBOutlet NSSecureTextField *fieldPin;
    @property (assign) IBOutlet NSSecureTextField *fieldPinConfirm;
    @property (assign) IBOutlet NSSecureTextField *fieldPinOld;
    @property (assign) IBOutlet NSButton          *buttonSetPin;
    @property (assign) IBOutlet NSButton          *buttonChangePin;
    @property (assign) IBOutlet NSButton          *buttonCancel;

@end

@implementation SetPinParamWindow

    - (void)windowDidLoad {
        [super windowDidLoad];

        // 画面項目を初期化
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

    - (IBAction)buttonChangePinDidPress:(id)sender {
        [self doChangePin:sender];
    }

    - (IBAction)buttonSetPinDidPress:(id)sender {
        [self doChangePin:sender];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // 画面項目を初期化し、この画面を閉じる
        [self initFieldValue];
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

#pragma mark - Check for entries and process

    - (void) doChangePin:(id)sender {
        // 入力内容チェック
        if ([self checkEntries:sender] == false) {
            return;
        }
    }

    - (bool) checkEntries:(id)sender {
        // 必須入力チェック
        if ([ToolCommon checkMustEntry:[self fieldPin]
                            informativeText:MSG_PROMPT_INPUT_NEW_PIN] == false) {
            return false;
        }
        if ([ToolCommon checkMustEntry:[self fieldPinConfirm]
                            informativeText:MSG_PROMPT_INPUT_NEW_PIN_CONFIRM] == false) {
            return false;
        }
        // 数字チェック
        if ([ToolCommon checkIsNumeric:[self fieldPin]
                       informativeText:MSG_PROMPT_INPUT_NEW_PIN_NUM] == false) {
            return false;
        }
        if ([ToolCommon checkIsNumeric:[self fieldPinConfirm]
                       informativeText:MSG_PROMPT_INPUT_NEW_PIN_CONF_NUM] == false) {
            return false;
        }
        // 変更ボタンがクリックされた場合は、変更前PINコードの入力チェックを実行
        if (sender == _buttonChangePin) {
            if ([ToolCommon checkMustEntry:[self fieldPinOld]
                           informativeText:MSG_PROMPT_INPUT_OLD_PIN] == false) {
                return false;
            }
            if ([ToolCommon checkIsNumeric:[self fieldPinOld]
                          informativeText:MSG_PROMPT_INPUT_OLD_PIN_NUM] == false) {
                return false;
            }
        }
        // 確認用PINコードのチェック
        if ([ToolCommon compareEntry:[self fieldPinConfirm] srcField:[self fieldPin]
                     informativeText:MSG_PROMPT_INPUT_PIN_CONFIRM_CRCT] == false) {
            return false;
        }
        return true;
    }

@end
