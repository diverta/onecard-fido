//
//  CTAP2ManageCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/19.
//
#ifndef CTAP2ManageCommand_h
#define CTAP2ManageCommand_h

#import <Foundation/Foundation.h>

@protocol CTAP2ManageCommandDelegate;

@interface CTAP2ManageCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (bool)isUSBHIDConnected;
    - (void)doRequestHidCtap2Management:(id)parameterRef;

@end

@protocol CTAP2ManageCommandDelegate <NSObject>

    - (void)notifyMessage:(NSString *)message;
    - (void)doResponseCtap2Management:(bool)success message:(NSString *)message;

@end

#endif /* CTAP2ManageCommand_h */
