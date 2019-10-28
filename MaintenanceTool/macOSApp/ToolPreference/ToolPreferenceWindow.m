//
//  ToolPreferenceWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/10/25.
//
#import "ToolPreferenceWindow.h"
#import "ToolPopupWindow.h"
#import "ToolCommon.h"
#import "ToolCommonMessage.h"

@interface ToolPreferenceWindow ()

    @property (assign) IBOutlet NSTextField     *fieldServiceUUIDString;
    @property (assign) IBOutlet NSTextField     *fieldServiceUUIDScanSec;
    @property (assign) IBOutlet NSButton        *buttonAuthParamGet;
    @property (assign) IBOutlet NSButton        *buttonAuthParamSet;
    @property (assign) IBOutlet NSButton        *buttonAuthParamReset;
    @property (assign) IBOutlet NSButton        *buttonClose;

@end

@implementation ToolPreferenceWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        
        // 画面項目を初期化
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // 画面項目を初期値に設定し、設定書込・解除ボタンを押下不可とする
        [self initAuthParamFieldsAndButtons];
    }

    - (void)enableButtons:(bool)enabled {
        // ボタンや入力欄の使用可能／不可制御
        [[self buttonAuthParamGet] setEnabled:enabled];
        [[self buttonAuthParamSet] setEnabled:enabled];
        [[self buttonAuthParamReset] setEnabled:enabled];
        [[self buttonClose] setEnabled:enabled];
        
        [[self fieldServiceUUIDString] setEnabled:enabled];
        [[self fieldServiceUUIDScanSec] setEnabled:enabled];
    }

    - (IBAction)buttonAuthParamGetDidPress:(id)sender {
        [self doAuthParamGet:sender];
    }

    - (IBAction)buttonAuthParamSetDidPress:(id)sender {
        [self doAuthParamSet:sender];
    }

    - (IBAction)buttonAuthParamResetDidPress:(id)sender {
        [self doAuthParamReset:sender];
    }

    - (IBAction)buttonCloseDidPress:(id)sender {
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // 画面項目を初期化し、この画面を閉じる
        [self initFieldValue];
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

#pragma mark - Interface for main process

    - (void)toolPreferenceCommandDidProcess:(ToolPreferenceCommandType)commandType
                                    success:(bool)success message:(NSString *)message {
        switch (commandType) {
            case COMMAND_AUTH_PARAM_GET:
                // 取得したパラメーターを画面項目に設定し、設定書込・解除ボタンを押下可とする
                [self setupAuthParamFieldsAndButtons];
                break;
            default:
                break;
        }
    }

#pragma mark - Subroutines

    - (void)initAuthParamFieldsAndButtons {
        // 画面項目をブランクに設定・使用不可とする
        [[self fieldServiceUUIDString] setStringValue:@""];
        [[self fieldServiceUUIDScanSec] setStringValue:@""];
        [[self fieldServiceUUIDString] setEnabled:false];
        [[self fieldServiceUUIDScanSec] setEnabled:false];

        // 設定書込・解除ボタンを押下不可とする
        [[self buttonAuthParamSet] setEnabled:false];
        [[self buttonAuthParamReset] setEnabled:false];
    }

    - (void)setupAuthParamFieldsAndButtons {
        // 画面項目に設定（スキャン対象サービスUUID）
        NSString *strUUID = [[self toolPreferenceCommand] serviceUUIDString];
        [[self fieldServiceUUIDString] setStringValue:strUUID];
        [[self fieldServiceUUIDString] setEnabled:true];

        // 画面項目に設定（スキャン秒数）
        NSString *strSec = [NSString stringWithFormat:@"%d",
                            [[self toolPreferenceCommand] serviceUUIDScanSec]];
        [[self fieldServiceUUIDScanSec] setStringValue:strSec];
        [[self fieldServiceUUIDScanSec] setEnabled:true];

        // 設定書込・解除ボタンを押下可とする
        [[self buttonAuthParamSet] setEnabled:true];
        [[self buttonAuthParamReset] setEnabled:true];

        // 最初の項目にフォーカス
        [[self fieldServiceUUIDString] becomeFirstResponder];
    }

#pragma mark - Check for entries and process

    - (void) doAuthParamGet:(id)sender {
        // 自動認証用パラメーター照会コマンドを実行し、スキャン対象サービスUUID、スキャン秒数を読込
        [[self toolPreferenceCommand] commandWillProcess:COMMAND_AUTH_PARAM_GET];
    }

    - (void) doAuthParamSet:(id)sender {
        // 入力内容チェック
        if ([self checkEntries:sender] == false) {
            return;
        }
        // スキャン対象サービスUUID、スキャン秒数を設定し、自動認証用パラメーター設定コマンドを実行
        [[self toolPreferenceCommand] commandWillProcess:COMMAND_AUTH_PARAM_SET];
    }

    - (void) doAuthParamReset:(id)sender {
        // 処理続行確認ダイアログを開く
        if ([ToolPopupWindow promptYesNo:MSG_CLEAR_UUID_SCAN_PARAM
                         informativeText:MSG_PROMPT_CLEAR_UUID_SCAN_PARAM] == false) {
            return;
        }
        // 自動認証用パラメーター解除コマンドを実行
        [[self toolPreferenceCommand] commandWillProcess:COMMAND_AUTH_PARAM_RESET];
    }

    - (bool) checkEntries:(id)sender {
        // 長さチェック
        if ([ToolCommon checkEntrySize:[self fieldServiceUUIDString]
                               minSize:UUID_STRING_SIZE maxSize:UUID_STRING_SIZE
                       informativeText:MSG_PROMPT_INPUT_UUID_STRING_LEN] == false) {
            return false;
        }
        if ([ToolCommon checkEntrySize:[self fieldServiceUUIDScanSec]
                               minSize:UUID_SCAN_SEC_SIZE maxSize:UUID_SCAN_SEC_SIZE
                       informativeText:MSG_PROMPT_INPUT_UUID_SCAN_SEC_LEN] == false) {
            return false;
        }
        // 数字チェック
        if ([ToolCommon checkIsNumeric:[self fieldServiceUUIDScanSec]
                       informativeText:MSG_PROMPT_INPUT_UUID_SCAN_SEC_NUM] == false) {
            return false;
        }
        // 範囲チェック
        if ([ToolCommon checkValueInRange:[self fieldServiceUUIDScanSec]
                                 minValue:1 maxValue:9
                          informativeText:MSG_PROMPT_INPUT_UUID_SCAN_SEC_RANGE] == false) {
            return false;
        }
        // 入力形式チェック（正規表現チェック）
        NSString *pattern = @"([0-9a-fA-F]{8}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{12})";
        if ([ToolCommon checkValueWithPattern:[self fieldServiceUUIDString]
                                      pattern:pattern
                              informativeText:MSG_PROMPT_INPUT_UUID_STRING_PATTERN] == false) {
            return false;
        }
        return true;
    }

@end
