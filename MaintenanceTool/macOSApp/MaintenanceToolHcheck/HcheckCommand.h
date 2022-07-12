//
//  HcheckCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/12.
//
#ifndef HcheckCommand_h
#define HcheckCommand_h

#import "AppCommand.h"

@interface HcheckCommandParameter : NSObject

    @property (nonatomic) Command       command;
    @property (nonatomic) NSString     *pin;

@end

@interface HcheckCommand : AppCommand

    - (void)hcheckWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (bool)isUSBHIDConnected;

@end

#endif /* HcheckCommand_h */
