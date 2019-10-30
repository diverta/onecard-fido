#ifndef AppDelegate_h
#define AppDelegate_h

#import <Cocoa/Cocoa.h>
#import "ToolCommon.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>

#pragma mark - Call back from other class
    - (void)toolPreferenceWillProcess:(Command)command withData:(NSData *)data;
    - (void)toolPreferenceWindowDidClose;

@end

#endif /* AppDelegate_h */
