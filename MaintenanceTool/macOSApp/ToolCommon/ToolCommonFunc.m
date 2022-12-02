//
//  ToolCommonFunc.m
//  ToolCommon
//
//  Created by Makoto Morita on 2022/06/08.
//
#import "FIDODefines.h"
#import "ToolCommonMessage.h"
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

    + (bool)checkUSBHIDConnectionOnWindow:(NSWindow *)window connected:(bool)connected {
        // USBポートに接続されていない場合はfalse
        if (connected == false) {
            [[ToolPopupWindow defaultWindow] critical:MSG_PROMPT_USB_PORT_SET informativeText:nil
                                           withObject:nil forSelector:nil parentWindow:window];
            return false;
        }
        return true;
    }

#pragma mark - Utilities for maintenance commands

    + (NSData *)commandDataForEraseBondingData {
        // ペアリング情報削除コマンド用のデータを生成
        unsigned char arr[] = {MNT_COMMAND_ERASE_BONDING_DATA};
        NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
        return commandData;
    }

    + (NSData *)commandDataForChangeToBootloaderMode {
        // ブートローダーモード遷移コマンド用のデータを生成
        unsigned char arr[] = {MNT_COMMAND_BOOTLOADER_MODE};
        NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
        return commandData;
    }

    + (NSData *)commandDataForSystemReset {
        // ファームウェアリセットコマンド用のデータを生成
        unsigned char arr[] = {MNT_COMMAND_SYSTEM_RESET};
        NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
        return commandData;
    }

    + (NSData *)commandDataForGetFlashStat {
        // Flash ROM情報照会コマンド用のデータを生成
        unsigned char arr[] = {MNT_COMMAND_GET_FLASH_STAT};
        NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
        return commandData;
    }

    + (NSData *)commandDataForGetVersionInfo {
        // バージョン照会コマンド用のデータを生成
        unsigned char arr[] = {MNT_COMMAND_GET_APP_VERSION};
        NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
        return commandData;
    }

@end
