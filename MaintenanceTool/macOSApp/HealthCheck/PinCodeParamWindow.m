//
//  PinCodeParamWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/02/27.
//
#import "PinCodeParamWindow.h"
#import "ToolCommon.h"
#import "ToolCommonMessage.h"

@interface PinCodeParamWindow ()

    @property (assign) IBOutlet NSSecureTextField *fieldPin;
    @property (assign) IBOutlet NSButton          *buttonOK;
    @property (assign) IBOutlet NSButton          *buttonCancel;

@end

@implementation PinCodeParamWindow

    - (void)windowDidLoad {
        [super windowDidLoad];

        // 画面項目を初期化
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // 画面項目を初期値に設定
        [[self fieldPin]        setStringValue:@""];

        // 最初の項目にフォーカス
        [[self fieldPin] becomeFirstResponder];
    }

    - (IBAction)buttonOKDidPress:(id)sender {
        [self doOK:sender];
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

    - (void) doOK:(id)sender {
        // 入力内容チェック
        if ([self checkEntries:sender] == false) {
            return;
        }
        // PINコード、実行コマンドを設定して画面を閉じる
        [[self toolCTAP2HealthCheckCommand] setPinCur:[[self fieldPin] stringValue]];
        [self terminateWindow:NSModalResponseOK];
    }

    - (bool) checkEntries:(id)sender {
        // 長さチェック
        if ([ToolCommon checkEntrySize:[self fieldPin]
                               minSize:PIN_CODE_SIZE_MIN maxSize:PIN_CODE_SIZE_MAX
                       informativeText:MSG_PROMPT_INPUT_CUR_PIN] == false) {
            return false;
        }
        // 数字チェック
        if ([ToolCommon checkIsNumeric:[self fieldPin]
                       informativeText:MSG_PROMPT_INPUT_CUR_PIN_NUM] == false) {
            return false;
        }
        return true;
    }

@end
