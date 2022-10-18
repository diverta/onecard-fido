//
//  USBDFUCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/18.
//
#ifndef USBDFUCommand_h
#define USBDFUCommand_h

#import "AppDefine.h"

@protocol USBDFUCommandDelegate;

@interface USBDFUCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)usbDfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (bool)isUSBHIDConnected;

@end

@protocol USBDFUCommandDelegate <NSObject>

    - (void)notifyCommandStartedWithCommand:(Command)command;
    - (void)notifyMessage:(NSString *)message;
    - (void)notifyCommandTerminated:(Command)command success:(bool)success message:(NSString *)message;

@end

#endif /* USBDFUCommand_h */
