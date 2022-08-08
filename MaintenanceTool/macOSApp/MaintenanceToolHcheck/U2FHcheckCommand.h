//
//  U2FHcheckCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/13.
//
#ifndef U2FHcheckCommand_h
#define U2FHcheckCommand_h

#import <Foundation/Foundation.h>

@protocol U2FHcheckCommandDelegate;

@interface U2FHcheckCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (bool)isUSBHIDConnected;
    - (void)doRequestHidU2fHealthCheck;
    - (void)doRequestBleU2fHealthCheck;
    - (void)doRequestHidPingTest;
    - (void)doRequestBlePingTest;

@end

@protocol U2FHcheckCommandDelegate <NSObject>

    - (void)notifyMessage:(NSString *)message;
    - (void)doResponseU2fHealthCheck:(bool)success message:(NSString *)message;

@end

#endif /* U2FHcheckCommand_h */
