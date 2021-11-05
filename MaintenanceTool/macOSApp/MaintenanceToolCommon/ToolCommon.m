//
//  ToolCommon.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/03/05.
//
#import <Foundation/Foundation.h>
#import "debug_log.h"
#import "ToolCommon.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"
#import "ToolPopupWindow.h"

// for SHA-256 hash calculate
#include <CommonCrypto/CommonCrypto.h>

@interface ToolCommon ()

@end

@implementation ToolCommon

    + (NSString *)getAppVersionString {
        return [[NSBundle mainBundle] infoDictionary][@"CFBundleShortVersionString"];
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

    + (NSData *)generateSHA256HashDataOf:(NSData *)data {
        uint8_t hash[32];
        uint8_t *dataBytes = (uint8_t *)[data bytes];
        CC_SHA256(dataBytes, (CC_LONG)[data length], hash);

        NSData *hashData = [[NSData alloc] initWithBytes:hash length:sizeof(hash)];
        return hashData;
    }

    + (void)setLENumber16:(uint16_t)n toBEBytes:(uint8_t *)p {
        // 指定領域から２バイト分の領域に、数値データをビッグエンディアン形式で設定
        p[0] = n >> 8 & 0xff;
        p[1] = n >> 0 & 0xff;
    }

    + (void)setLENumber32:(uint32_t)n toBEBytes:(uint8_t *)p {
        // 指定領域から４バイト分の領域に、数値データをビッグエンディアン形式で設定
        p[0] = n >> 24 & 0xff;
        p[1] = n >> 16 & 0xff;
        p[2] = n >>  8 & 0xff;
        p[3] = n >>  0 & 0xff;
    }

    + (uint16_t)getLENumber16FromBEBytes:(uint8_t *)p {
        // 指定領域から２バイト分の領域を、リトルエンディアン形式で数値データに変換
        uint16_t uint16;
        uint8_t *q = (uint8_t *)&uint16;
        q[0] = p[1];
        q[1] = p[0];
        return uint16;
    }

    + (uint32_t)getLENumber32FromBEBytes:(uint8_t *)p {
        // 指定領域から４バイト分の領域を、リトルエンディアン形式で数値データに変換
        uint32_t uint32;
        uint8_t *q = (uint8_t *)&uint32;
        q[0] = p[3];
        q[1] = p[2];
        q[2] = p[1];
        q[3] = p[0];
        return uint32;
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

    + (bool) checkValueInRange:(NSTextField *)textField
                      minValue:(int)minValue maxValue:(int)maxValue
               informativeText:(NSString *)informativeText {
        // 入力項目が正しく指定されていない場合はfalseを戻す
        int value = [textField intValue];
        if (value < minValue || value > maxValue) {
            [ToolPopupWindow warning:MSG_INVALID_OUT_OF_RANGE informativeText:informativeText];
            [textField becomeFirstResponder];
            return false;
        }
        return true;
    }

    + (bool) checkValueWithPattern:(NSTextField *)textField
                           pattern:(NSString *)pattern
                   informativeText:(NSString *)informativeText {
        // 入力項目が正規表現にマッチしていない場合はfalseを戻す
        NSString *value = [textField stringValue];
        NSError *error = nil;
        NSRegularExpression* regex =
        [NSRegularExpression regularExpressionWithPattern:pattern
                                                  options:NSRegularExpressionCaseInsensitive
                                                    error:&error];
        NSTextCheckingResult *match =
        [regex firstMatchInString:value options:0 range:NSMakeRange(0, value.length)];
        
        if (match == nil) {
            [ToolPopupWindow warning:MSG_INVALID_PATTERN informativeText:informativeText];
            [textField becomeFirstResponder];
            return false;
        }
        return true;
    }

#pragma mark - Utilities for log output

    + (void)logErrorMessageWithFuncError:(NSString *)errorMsgTemplate {
        NSString *functionMsg = [[NSString alloc] initWithUTF8String:log_debug_message()];
        NSString *errorMsg = [[NSString alloc] initWithFormat:errorMsgTemplate, functionMsg];
        [[ToolLogFile defaultLogger] error:errorMsg];
    }

#pragma mark - Utilities for others

    + (NSArray<NSString *> *)extractValuesFromVersionInfo:(NSString *)versionInfoCSV {
        // 情報取得CSVからバージョン情報を抽出
        NSString *strDeviceName = @"";
        NSString *strFWRev = @"";
        NSString *strHWRev = @"";
        NSString *strSecic = @"";
        for (NSString *element in [versionInfoCSV componentsSeparatedByString:@","]) {
            NSArray *items = [element componentsSeparatedByString:@"="];
            NSString *key = [items objectAtIndex:0];
            NSString *val = [items objectAtIndex:1];
            if ([key isEqualToString:@"DEVICE_NAME"]) {
                strDeviceName = [ToolCommon extractCSVItemFrom:val];
            } else if ([key isEqualToString:@"FW_REV"]) {
                strFWRev = [ToolCommon extractCSVItemFrom:val];
            } else if ([key isEqualToString:@"HW_REV"]) {
                strHWRev = [ToolCommon extractCSVItemFrom:val];
            } else if ([key isEqualToString:@"ATECC608A"]) {
                strSecic = [ToolCommon extractCSVItemFrom:val];
            }
        }
        return @[strDeviceName, strFWRev, strHWRev, strSecic];
    }

    + (NSData *)extractCBORBytesFrom:(NSData *)responseMessage {
        // CBORバイト配列（レスポンスの２バイト目以降）を抽出
        size_t cborLength = [responseMessage length] - 1;
        NSData *cborBytes = [responseMessage subdataWithRange:NSMakeRange(1, cborLength)];
        return cborBytes;
    }

    + (NSString *)extractCSVItemFrom:(NSString *)val {
        // 文字列の前後に２重引用符が含まれていない場合は終了
        if ([val length] < 2) {
            return val;
        }
        // 取得した項目から、２重引用符を削除
        NSString *item = [val stringByReplacingOccurrencesOfString:@"\"" withString:@""];
        return item;
    }

    + (int)calculateDecimalVersion:(NSString *)versionStr {
        // バージョン文字列 "1.2.11" -> "010211" 形式に変換
        int decimalVersion = 0;
        for (NSString *element in [versionStr componentsSeparatedByString:@"."]) {
            decimalVersion = decimalVersion * 100 + [element intValue];
        }
        return decimalVersion;
    }

@end
