//
//  BLESettingCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/19.
//
#ifndef BLESettingCommand_h
#define BLESettingCommand_h

#import "AppCommand.h"

@interface BLESettingCommandParameter : NSObject

    @property (nonatomic) Command       command;

@end

@interface BLESettingCommand : AppCommand

    - (void)bleSettingWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (bool)isUSBHIDConnected;

@end

#endif /* BLESettingCommand_h */
