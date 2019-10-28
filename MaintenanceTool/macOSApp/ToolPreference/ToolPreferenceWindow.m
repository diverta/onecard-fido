//
//  ToolPreferenceWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/10/25.
//
#import "ToolPreferenceWindow.h"
#import "ToolCommon.h"
#import "ToolCommonMessage.h"

@interface ToolPreferenceWindow ()

    @property (assign) IBOutlet NSTextField     *fieldServiceUUIDString;
    @property (assign) IBOutlet NSTextField     *fieldServiceUUIDScanSec;
    @property (assign) IBOutlet NSButton        *buttonAuthParamGet;
    @property (assign) IBOutlet NSButton        *buttonAuthParamSet;
    @property (assign) IBOutlet NSButton        *buttonClose;

@end

@implementation ToolPreferenceWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        
        // 画面項目を初期化
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // 画面項目を初期値に設定
        [[self fieldServiceUUIDString] setStringValue:@""];
        
        // 最初の項目にフォーカス
        [[self fieldServiceUUIDString] becomeFirstResponder];
    }

    - (IBAction)buttonAuthParamGetDidPress:(id)sender {
        [self doAuthParamGet:sender];
    }

    - (IBAction)buttonAuthParamSetDidPress:(id)sender {
        [self doAuthParamSet:sender];
    }

    - (IBAction)buttonCloseDidPress:(id)sender {
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // 画面項目を初期化し、この画面を閉じる
        [self initFieldValue];
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

#pragma mark - Check for entries and process

    - (void) doAuthParamGet:(id)sender {
        // 自動認証用パラメーター照会コマンドを実行し、スキャン対象サービスUUID、スキャン秒数を読込み、各画面項目に設定
        // TODO
    }

    - (void) doAuthParamSet:(id)sender {
        // 入力内容チェック
        if ([self checkEntries:sender] == false) {
            return;
        }
        // スキャン対象サービスUUID、スキャン秒数を設定し、自動認証用パラメーター設定コマンドを実行
        // TODO
    }

    - (bool) checkEntries:(id)sender {
        // 長さチェック
        if ([ToolCommon checkEntrySize:[self fieldServiceUUIDString]
                               minSize:PIN_CODE_SIZE_MIN maxSize:PIN_CODE_SIZE_MAX
                       informativeText:MSG_PROMPT_INPUT_CUR_PIN] == false) {
            return false;
        }
        if ([ToolCommon checkEntrySize:[self fieldServiceUUIDScanSec]
                               minSize:PIN_CODE_SIZE_MIN maxSize:PIN_CODE_SIZE_MAX
                       informativeText:MSG_PROMPT_INPUT_CUR_PIN] == false) {
            return false;
        }
        // 数字チェック
        if ([ToolCommon checkIsNumeric:[self fieldServiceUUIDScanSec]
                       informativeText:MSG_PROMPT_INPUT_CUR_PIN_NUM] == false) {
            return false;
        }
        return true;
    }

@end
