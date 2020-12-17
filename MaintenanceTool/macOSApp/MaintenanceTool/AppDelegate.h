#ifndef AppDelegate_h
#define AppDelegate_h

#import <Cocoa/Cocoa.h>
#import "ToolCommon.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>


#pragma mark - Call from other class

    - (id)toolInfoWindowRef;
    - (bool)checkUSBHIDConnection;
    - (void)toolPreferenceWillProcess:(Command)command withData:(NSData *)data;
    - (void)toolPreferenceWindowDidClose;
    - (void)toolPreferenceInquiryDidProcess:(Command)command
                                        CMD:(uint8_t)cmd response:(NSData *)resp
                                     result:(bool)result message:(NSString *)message;
    - (void)toolDFUCommandDidStart;
    - (void)toolDFUCommandDidTerminate:(Command)command result:(bool)result message:(NSString *)message;
    - (void)toolPIVCommandDidTerminate:(Command)command result:(bool)result message:(NSString *)message;

@end

#endif /* AppDelegate_h */
