//
//  FIDOSettingCommand.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/08.
//
#ifndef FIDOSettingCommand_h
#define FIDOSettingCommand_h

#import "AppCommand.h"

@interface FIDOSettingCommand : AppCommand

    - (void)FIDOSettingWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (bool)checkUSBHIDConnection;

@end

#endif /* FIDOSettingCommand_h */
