#ifndef AppDelegate_h
#define AppDelegate_h

#import <Cocoa/Cocoa.h>
#import "ToolCommon.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>


#pragma mark - Call from other class

    - (bool)checkUSBHIDConnection;
    - (void)toolPreferenceWillProcess:(Command)command withData:(NSData *)data;
    - (void)toolPreferenceWindowDidClose;
    - (void)toolDFUCommandDidStart;
    - (void)toolDFUCommandDidTerminate:(Command)command result:(bool)result message:(NSString *)message;

@end

#endif /* AppDelegate_h */
