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
    COMMAND_BLE_GET_VERSION_INFO,
    COMMAND_HID_GET_FLASH_STAT,
    COMMAND_HID_GET_VERSION_INFO,
    COMMAND_HID_GET_VERSION_FOR_DFU,
    COMMAND_HID_BOOTLOADER_MODE,
    COMMAND_HID_FIRMWARE_RESET,
    COMMAND_CLIENT_PIN_SET,
    COMMAND_CLIENT_PIN_CHANGE,
    COMMAND_TEST_MAKE_CREDENTIAL,
    COMMAND_TEST_GET_ASSERTION,
    COMMAND_AUTH_RESET,
    COMMAND_TOOL_PREF_PARAM,
    COMMAND_TOOL_PREF_PARAM_INQUIRY,
    COMMAND_USB_DFU,
    COMMAND_BLE_DFU,
    COMMAND_BLE_DFU_GET_SLOT_INFO,
    COMMAND_BLE_DFU_UPLOAD_IMAGE,
    COMMAND_BLE_DFU_CHANGE_IMAGE_UPDATE_MODE,
    COMMAND_BLE_DFU_RESET_APPLICATION,
    COMMAND_ERASE_BONDS,
    COMMAND_CCID_PIV_CHANGE_PIN,
    COMMAND_CCID_PIV_CHANGE_PUK,
    COMMAND_CCID_PIV_UNBLOCK_PIN,
    COMMAND_CCID_PIV_RESET,
    COMMAND_CCID_PIV_IMPORT_KEY,
    COMMAND_CCID_PIV_SET_CHUID,
    COMMAND_CCID_PIV_STATUS,
    COMMAND_OPENPGP_INSTALL_KEYS,
    COMMAND_OPENPGP_STATUS,
    COMMAND_OPENPGP_RESET,
    COMMAND_OPENPGP_CHANGE_PIN,
    COMMAND_OPENPGP_CHANGE_ADMIN_PIN,
    COMMAND_OPENPGP_UNBLOCK_PIN,
    COMMAND_OPENPGP_SET_RESET_CODE,
    COMMAND_OPENPGP_UNBLOCK,
    COMMAND_OPEN_WINDOW_FIDOATTEST,
    COMMAND_OPEN_WINDOW_PINPARAM,
    COMMAND_OPEN_WINDOW_PIVPARAM,
    COMMAND_OPEN_WINDOW_PGPPARAM,
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
    + (NSData *)generateSHA256HashDataOf:(NSData *)data;
    + (void)setLENumber16:(uint16_t)n toBEBytes:(uint8_t *)p;
    + (void)setLENumber32:(uint32_t)n toBEBytes:(uint8_t *)p;
    + (uint16_t)getLENumber16FromBEBytes:(uint8_t *)p;
    + (uint32_t)getLENumber32FromBEBytes:(uint8_t *)p;

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

    + (void)logErrorMessageWithFuncError:(NSString *)errorMsgTemplate;

    + (NSArray<NSString *> *)extractValuesFromVersionInfo:(NSString *)versionInfoCSV;
    + (NSData *)extractCBORBytesFrom:(NSData *)responseMessage;
    + (NSString *)extractCSVItemFrom:(NSString *)val;
    + (int)calculateDecimalVersion:(NSString *)versionStr;

@end


#endif /* ToolCommon_h */
