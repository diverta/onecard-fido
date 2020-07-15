//
//  ToolPreferenceCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/10/24.
//
#ifndef ToolPreferenceCommand_h
#define ToolPreferenceCommand_h

#import "AppDelegate.h"
#import "ToolCommon.h"

// サービスUUID、スキャン秒数の桁数（固定）
#define UUID_STRING_SIZE    36
#define UUID_SCAN_SEC_SIZE  1

// 設定画面のコマンド種別
typedef enum : NSInteger {
    COMMAND_AUTH_PARAM_GET = 1,
    COMMAND_AUTH_PARAM_SET,
    COMMAND_AUTH_PARAM_RESET
} ToolPreferenceCommandType;

@interface ToolPreferenceCommand : NSObject

    // 認証器の設定値を保持
    @property (nonatomic) bool      bleScanAuthEnabled;
    @property (nonatomic) NSString  *serviceUUIDString;
    @property (nonatomic) NSString  *serviceUUIDScanSec;

    - (id)initWithDelegate:(id)delegate;
    - (void)toolPreferenceWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;

    // AppDelegate経由でコマンドを実行
    - (void)toolPreferenceWillProcess:(ToolPreferenceCommandType)commandType;
    - (void)toolPreferenceInquiryWillProcess;

    // AppDelegate経由で実行したコマンドの応答
    - (void)toolPreferenceDidProcess:(Command)command
                                 CMD:(uint8_t)cmd response:(NSData *)resp
                              result:(bool)result message:(NSString *)message;

@end

#endif /* ToolPreferenceCommand_h */
