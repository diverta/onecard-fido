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

@interface ToolPreferenceCommand : NSObject

    @property (nonatomic, weak) id  delegate;

    - (id)initWithDelegate:(id)delegate;
    - (void)setTransportParam:(TransportType)type
               toolBLECommand:(ToolBLECommand *)ble toolHIDCommand:(ToolHIDCommand *)hid;
    - (void)toolPreferenceWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;

@end

#endif /* ToolPreferenceCommand_h */
