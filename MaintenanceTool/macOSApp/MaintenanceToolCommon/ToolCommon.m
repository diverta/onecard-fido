//
//  ToolCommon.m
//  MaintenanceTool
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
            case COMMAND_TEST_CTAPHID_PING:
                processName = PROCESS_NAME_TEST_CTAPHID_PING;
                break;
            case COMMAND_TEST_BLE_PING:
                processName = PROCESS_NAME_TEST_BLE_PING;
                break;
            case COMMAND_HID_GET_FLASH_STAT:
                processName = PROCESS_NAME_GET_FLASH_STAT;
                break;
            case COMMAND_CLIENT_PIN_SET:
                processName = PROCESS_NAME_CLIENT_PIN_SET;
                break;
            case COMMAND_CLIENT_PIN_CHANGE:
                processName = PROCESS_NAME_CLIENT_PIN_CHANGE;
                break;
            case COMMAND_TEST_MAKE_CREDENTIAL:
            case COMMAND_TEST_GET_ASSERTION:
                processName = PROCESS_NAME_CTAP2_HEALTHCHECK;
                break;
            case COMMAND_AUTH_RESET:
                processName = PROCESS_NAME_AUTH_RESET;
                break;
            default:
                processName = nil;
                break;
        }
        return processName;
    }

    + (NSData *)generateHexBytesFrom:(NSString *)hexString {
        unsigned int  hexInt;
        unsigned char byte;
        
        // 与えられたHEX文字列を２文字ずつ切り出し、バイトデータに変換する
        NSMutableData *convertedBytes = [[NSMutableData alloc] init];
        for (int i = 0; i < [hexString length]; i+=2) {
            NSString *tmp = [hexString substringWithRange:NSMakeRange(i, 2)];
            [[NSScanner scannerWithString:tmp] scanHexInt:&hexInt];
            byte = (unsigned char)hexInt;
            [convertedBytes appendBytes:&byte length:sizeof(byte)];
        }
        
        return convertedBytes;
    }

    + (NSData *)generateRandomBytesDataOf:(size_t)size {
        unsigned char *randomBytes = (unsigned char *)malloc(size);
        
        for (int i = 0; i < size; i++) {
            // from 0 to 255
            randomBytes[i] = (unsigned char)arc4random_uniform(256);
        }
        NSData *randomBytesData = [[NSData alloc] initWithBytes:randomBytes length:size];
        free(randomBytes);

        return randomBytesData;
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

    + (bool) checkIsNumeric:(NSTextField *)textField informativeText:(NSString *)informativeText{
        // 入力値が数字だけで構成されていない場合はfalseを戻す
        NSString *string = [textField stringValue];
        NSCharacterSet *characterSet = [NSCharacterSet characterSetWithCharactersInString:string];
        if ([[NSCharacterSet decimalDigitCharacterSet] isSupersetOfSet:characterSet] == false) {
            [ToolPopupWindow warning:MSG_NOT_NUMERIC informativeText:informativeText];
            [textField becomeFirstResponder];
            return false;
        }
        return true;
    }

    + (bool) compareEntry:(NSTextField *)destField srcField:(NSTextField *)srcField
          informativeText:(NSString *)informativeText {
        // 入力項目が等しくない場合はfalseを戻す
        if ([[destField stringValue] isEqualToString:[srcField stringValue]] == false) {
            [ToolPopupWindow warning:MSG_INVALID_FIELD informativeText:informativeText];
            [destField becomeFirstResponder];
            return false;
        }
        return true;
    }

    + (bool) checkEntrySize:(NSTextField *)textField
                    minSize:(size_t)minSize maxSize:(size_t)maxSize
            informativeText:(NSString *)informativeText {
        // 入力項目が正しく指定されていない場合はfalseを戻す
        size_t size = [[textField stringValue] length];
        if (size < minSize || size > maxSize) {
            [ToolPopupWindow warning:MSG_INVALID_FIELD_SIZE informativeText:informativeText];
            [textField becomeFirstResponder];
            return false;
        }
        return true;
    }

@end
