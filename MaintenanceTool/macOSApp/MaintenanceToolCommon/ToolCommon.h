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

// コマンド種別
typedef enum : NSInteger {
    COMMAND_NONE = 1,
    COMMAND_ERASE_SKEY_CERT,
    COMMAND_INSTALL_SKEY_CERT,
    COMMAND_TEST_REGISTER,
    COMMAND_TEST_AUTH_CHECK,
    COMMAND_TEST_AUTH_NO_USER_PRESENCE,
    COMMAND_TEST_AUTH_USER_PRESENCE,
    COMMAND_PAIRING,
    COMMAND_TEST_CTAPHID_PING,
    COMMAND_TEST_BLE_PING,
    COMMAND_HID_GET_FLASH_STAT,
    COMMAND_HID_GET_VERSION_INFO,
    COMMAND_HID_GET_VERSION_FOR_DFU,
    COMMAND_HID_BOOTLOADER_MODE,
    COMMAND_CLIENT_PIN_SET,
    COMMAND_CLIENT_PIN_CHANGE,
    COMMAND_TEST_MAKE_CREDENTIAL,
    COMMAND_TEST_GET_ASSERTION,
    COMMAND_AUTH_RESET,
    COMMAND_TOOL_PREF_PARAM,
    COMMAND_TOOL_PREF_PARAM_INQUIRY,
    COMMAND_USB_DFU
} Command;

// トランスポート種別
typedef enum : NSInteger {
    TRANSPORT_NONE = 1,
    TRANSPORT_BLE,
    TRANSPORT_HID
} TransportType;

// ツールで共通利用する関数群
@interface ToolCommon : NSObject

    + (NSString *)getAppVersionString;
    + (NSData *)generateHexBytesFrom:(NSString *)hexString;
    + (NSData *)generateRandomBytesDataOf:(size_t)size;

    + (bool) checkMustEntry:(NSTextField *)textField informativeText:(NSString *)informativeText;
    + (bool) checkFileExist:(NSTextField *)textField informativeText:(NSString *)informativeText;
    + (bool) checkIsNumeric:(NSTextField *)textField informativeText:(NSString *)informativeText;
    + (bool) compareEntry:(NSTextField *)destField srcField:(NSTextField *)srcField
          informativeText:(NSString *)informativeText;
    + (bool) checkEntrySize:(NSTextField *)textField
                    minSize:(size_t)minSize maxSize:(size_t)maxSize
            informativeText:(NSString *)informativeText;
    + (bool) checkValueInRange:(NSTextField *)textField
                      minValue:(int)minValue maxValue:(int)maxValue
               informativeText:(NSString *)informativeText;
    + (bool) checkValueWithPattern:(NSTextField *)textField
                           pattern:(NSString *)pattern
                   informativeText:(NSString *)informativeText;

@end


#endif /* ToolCommon_h */
