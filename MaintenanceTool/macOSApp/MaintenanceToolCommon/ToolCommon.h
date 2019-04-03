//
//  ToolCommon.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/01/22.
//
#ifndef ToolCommon_h
#define ToolCommon_h

// BLE接続再試行の上限回数
#define BLE_CONNECTION_RETRY_MAX_COUNT 3

// コマンド種別
typedef enum : NSInteger {
    COMMAND_NONE = 1,
    COMMAND_ERASE_SKEY_CERT,
    COMMAND_INSTALL_SKEY_CERT,
    COMMAND_TEST_REGISTER,
    COMMAND_TEST_AUTH_CHECK,
    COMMAND_TEST_AUTH_NO_USER_PRESENCE,
    COMMAND_TEST_AUTH_USER_PRESENCE,
    COMMAND_U2F_HID_PROCESS,
    COMMAND_PAIRING,
    COMMAND_TEST_CTAPHID_INIT,
    COMMAND_CLIENT_PIN
} Command;

// ツールで共通利用する関数群
@interface ToolCommon : NSObject

    + (NSString *)processNameOfCommand:(Command)command;

    + (bool) checkMustEntry:(NSTextField *)textField informativeText:(NSString *)informativeText;
    + (bool) checkFileExist:(NSTextField *)textField informativeText:(NSString *)informativeText;
    + (bool) checkIsNumber:(NSTextField *)textField informativeText:(NSString *)informativeText;

@end


#endif /* ToolCommon_h */
