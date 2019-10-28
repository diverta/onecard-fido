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
#import "ToolBLECommand.h"
#import "ToolHIDCommand.h"

// 設定画面のコマンド種別
typedef enum : NSInteger {
    COMMAND_AUTH_PARAM_GET = 1,
    COMMAND_AUTH_PARAM_SET,
    COMMAND_AUTH_PARAM_RESET
} ToolPreferenceCommandType;

@interface ToolPreferenceCommand : NSObject

    @property (nonatomic, weak) id  delegate;

    - (id)initWithDelegate:(id)delegate;
    - (void)setTransportParam:(TransportType)type
               toolBLECommand:(ToolBLECommand *)ble toolHIDCommand:(ToolHIDCommand *)hid;
    - (void)toolPreferenceWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)toolPreferenceCommandWillProcess:(ToolPreferenceCommandType)commandType;

@end

#endif /* ToolPreferenceCommand_h */
