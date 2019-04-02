//
//  ToolCommon.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/03/05.
//
#import <Foundation/Foundation.h>

#import "ToolCommon.h"
#import "ToolCommonMessage.h"
#import "ToolPopupWindow.h"

@interface ToolCommon ()

@end

@implementation ToolCommon

    + (NSString *)processNameOfCommand:(Command)command {
        // コマンド種別に対応する処理名称を戻す
        NSString *processName;
        switch (command) {
            case COMMAND_PAIRING:
                processName = PROCESS_NAME_PAIRING;
                break;
            case COMMAND_ERASE_SKEY_CERT:
                processName = PROCESS_NAME_ERASE_SKEY_CERT;
                break;
            case COMMAND_INSTALL_SKEY_CERT:
                processName = PROCESS_NAME_INSTALL_SKEY_CERT;
                break;
            case COMMAND_TEST_REGISTER:
            case COMMAND_TEST_AUTH_CHECK:
            case COMMAND_TEST_AUTH_NO_USER_PRESENCE:
            case COMMAND_TEST_AUTH_USER_PRESENCE:
                processName = PROCESS_NAME_HEALTHCHECK;
                break;
            case COMMAND_TEST_CTAPHID_INIT:
                processName = PROCESS_NAME_TEST_CTAPHID_INIT;
                break;
            default:
                processName = nil;
                break;
        }
        return processName;
    }

#pragma mark - Utilities for check entry

    + (bool) checkMustEntry:(NSTextField *)textField informativeText:(NSString *)informativeText {
        // 入力項目が正しく指定されていない場合はfalseを戻す
        if ([[textField stringValue] length] == 0) {
            [ToolPopupWindow warning:MSG_INVALID_FIELD informativeText:informativeText];
            [textField becomeFirstResponder];
            return false;
        }
        return true;
    }

    + (bool) checkFileExist:(NSTextField *)textField informativeText:(NSString *)informativeText {
        // 入力されたファイルパスが存在しない場合はfalseを戻す
        if ([[NSFileManager defaultManager] fileExistsAtPath:[textField stringValue]] == false) {
            [ToolPopupWindow warning:MSG_INVALID_FILE_PATH informativeText:informativeText];
            [textField becomeFirstResponder];
            return false;
        }
        return true;
    }

    + (bool) checkIsNumber:(NSTextField *)textField informativeText:(NSString *)informativeText{
        // 入力値が数字だけで構成されていない場合はfalseを戻す
        NSString *string = [textField stringValue];
        NSCharacterSet *characterSet = [NSCharacterSet characterSetWithCharactersInString:string];
        if ([[NSCharacterSet decimalDigitCharacterSet] isSupersetOfSet:characterSet] == false) {
            [ToolPopupWindow warning:MSG_INVALID_NUMBER informativeText:informativeText];
            [textField becomeFirstResponder];
            return false;
        }
        return true;
    }

@end
