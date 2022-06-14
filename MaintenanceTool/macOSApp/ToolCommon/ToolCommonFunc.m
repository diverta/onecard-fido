//
//  ToolCommonFunc.m
//  ToolCommon
//
//  Created by Makoto Morita on 2022/06/08.
//
#import "AppCommonMessage.h"
#import "ToolCommonFunc.h"
#import "ToolPopupWindow.h"

@interface ToolCommonFunc ()

@end

@implementation ToolCommonFunc

    + (NSString *)getAppVersionString {
        return [[NSBundle mainBundle] infoDictionary][@"CFBundleShortVersionString"];
    }

#pragma mark - Utilities for check entry

    + (bool)checkMustEntry:(NSTextField *)textField informativeText:(NSString *)informativeText onWindow:(NSWindow *)window {
        // 入力項目が正しく指定されていない場合はfalseを戻す
        if ([[textField stringValue] length] == 0) {
            [[ToolPopupWindow defaultWindow] critical:MSG_INVALID_FIELD informativeText:informativeText withObject:nil forSelector:nil
                                         parentWindow:window];
            [textField becomeFirstResponder];
            return false;
        }
        return true;
    }

    + (bool)checkFileExist:(NSTextField *)textField informativeText:(NSString *)informativeText onWindow:(NSWindow *)window {
        // 入力されたファイルパスが存在しない場合はfalseを戻す
        if ([[NSFileManager defaultManager] fileExistsAtPath:[textField stringValue]] == false) {
            [[ToolPopupWindow defaultWindow] critical:MSG_INVALID_FILE_PATH informativeText:informativeText withObject:nil forSelector:nil
                                         parentWindow:window];
            [textField becomeFirstResponder];
            return false;
        }
        return true;
    }

@end
