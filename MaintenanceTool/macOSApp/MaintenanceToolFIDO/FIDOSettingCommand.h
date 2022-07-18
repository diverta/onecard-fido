//
//  FIDOSettingCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/18.
//
#ifndef FIDOSettingCommand_h
#define FIDOSettingCommand_h

#import "AppCommand.h"

@interface FIDOSettingCommandParameter : NSObject

    @property (nonatomic) Command       command;
    @property (nonatomic) NSString     *pinNew;
    @property (nonatomic) NSString     *pinOld;

@end

@interface FIDOSettingCommand : AppCommand

    - (void)fidoSettingWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (bool)isUSBHIDConnected;

@end

#endif /* FIDOSettingCommand_h */
