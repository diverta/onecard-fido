//
//  UtilityCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/06.
//
#ifndef UtilityCommand_h
#define UtilityCommand_h

#import "AppCommand.h"

@interface UtilityCommand : AppCommand

    - (void)utilityWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (bool)checkUSBHIDConnectionOnWindow:(NSWindow *)window;

@end

#endif /* UtilityCommand_h */
