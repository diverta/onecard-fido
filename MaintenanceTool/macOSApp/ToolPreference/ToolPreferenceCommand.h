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

    @property (nonatomic, weak) id  delegate;

    // 認証器の設定値を保持
    @property (nonatomic) NSString  *serviceUUIDString;
    @property (nonatomic) uint8_t   serviceUUIDScanSec;

    - (id)initWithDelegate:(id)delegate;
    - (void)toolPreferenceWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)commandWillProcess:(ToolPreferenceCommandType)commandType;

@end

#endif /* ToolPreferenceCommand_h */
