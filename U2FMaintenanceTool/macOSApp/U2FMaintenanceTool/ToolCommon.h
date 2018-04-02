//
//  ToolCommon.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/01/22.
//
#ifndef ToolCommon_h
#define ToolCommon_h

#define CHROME_EXTENSION_ID_URL @"chrome-extension://pfboblefjcgdjicmnffhdgionmgcdmne/"

// Defines for Chrome Native Messaging Host
#define CHROME_NMHOST_NAME     @"jp.co.diverta.chrome.helper.ble.u2f"
#define CHROME_NMHOST_DESC     @"BLE U2F Helper - Chrome Native Messaging Host"
#define CHROME_NMHOST_TYPE     @"stdio"
#define CHROME_NMHOST_JSON_DIR @"/Library/Application Support/Google/Chrome/NativeMessagingHosts"

// BLE接続再試行の上限回数
#define BLE_CONNECTION_RETRY_MAX_COUNT 3

// コマンド種別
typedef enum : NSInteger {
    COMMAND_NONE = 1,
    COMMAND_ERASE_BOND,
    COMMAND_ERASE_SKEY_CERT,
    COMMAND_INSTALL_SKEY,
    COMMAND_INSTALL_CERT,
    COMMAND_TEST_REGISTER,
    COMMAND_TEST_AUTH_CHECK,
    COMMAND_TEST_AUTH_NO_USER_PRESENCE,
    COMMAND_TEST_AUTH_USER_PRESENCE,
    COMMAND_U2F_PROCESS,
    COMMAND_SETUP_CHROME_NATIVE_MESSAGING,
    COMMAND_CREATE_KEYPAIR_PEM,
    COMMAND_CREATE_CERTREQ_CSR,
    COMMAND_CREATE_SELFCRT_CRT
} Command;

// ツールで共通利用する関数群
@interface ToolCommon : NSObject

    + (NSString *)processNameOfCommand:(Command)command;

@end


#endif /* ToolCommon_h */
