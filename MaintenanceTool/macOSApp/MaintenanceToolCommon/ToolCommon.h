//
//  ToolCommon.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/01/22.
//
#ifndef ToolCommon_h
#define ToolCommon_h

// BLE接続再試行の上限回数
#define BLE_CONNECTION_RETRY_MAX_COUNT 3

// PINコードの最小／最大桁数
#define PIN_CODE_SIZE_MIN 4
#define PIN_CODE_SIZE_MAX 16
#define PIV_PIN_CODE_SIZE_MIN 6
#define PIV_PIN_CODE_SIZE_MAX 8

// ツールで共通利用する関数群
@interface ToolCommon : NSObject

    + (NSData *)generateHexBytesFrom:(NSString *)hexString;
    + (NSData *)generateRandomBytesDataOf:(size_t)size;
    + (NSData *)generateSHA256HashDataOf:(NSData *)data;
    + (void)setLENumber16:(uint16_t)n toBEBytes:(uint8_t *)p;
    + (void)setLENumber32:(uint32_t)n toBEBytes:(uint8_t *)p;
    + (uint16_t)getLENumber16FromBEBytes:(uint8_t *)p;
    + (uint32_t)getLENumber32FromBEBytes:(uint8_t *)p;

    + (bool)checkIsNumeric:(NSTextField *)textField informativeText:(NSString *)informativeText onWindow:(NSWindow *)window;
    + (bool)compareEntry:(NSTextField *)destField srcField:(NSTextField *)srcField
          informativeText:(NSString *)informativeText onWindow:(NSWindow *)window;
    + (bool)checkEntrySize:(NSTextField *)textField
                   minSize:(size_t)minSize maxSize:(size_t)maxSize
           informativeText:(NSString *)informativeText onWindow:(NSWindow *)window;
    + (bool)checkValueInRange:(NSTextField *)textField
                    minValue:(int)minValue maxValue:(int)maxValue
            informativeText:(NSString *)informativeText onWindow:(NSWindow *)window;
    + (bool)checkValueWithPattern:(NSTextField *)textField
                        pattern:(NSString *)pattern
                informativeText:(NSString *)informativeText onWindow:(NSWindow *)window;

    + (void)logErrorMessageWithFuncError:(NSString *)errorMsgTemplate;

    + (NSArray<NSString *> *)extractValuesFromVersionInfo:(NSString *)versionInfoCSV;
    + (NSData *)extractCBORBytesFrom:(NSData *)responseMessage;
    + (NSString *)extractCSVItemFrom:(NSString *)val;
    + (int)calculateDecimalVersion:(NSString *)versionStr;

@end


#endif /* ToolCommon_h */
