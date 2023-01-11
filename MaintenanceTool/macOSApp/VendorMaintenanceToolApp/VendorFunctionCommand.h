//
//  VendorFunctionCommand.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2023/01/11.
//
#ifndef VendorFunctionCommand_h
#define VendorFunctionCommand_h

#import "AppCommand.h"

@interface VendorFunctionCommandParameter : NSObject

    @property (nonatomic) Command       command;

@end

@interface VendorFunctionCommand : AppCommand

    - (id)initWithDelegate:(id)delegate;
    - (bool)isUSBHIDConnected;
    - (void)vendorFunctionWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;

@end

#endif /* VendorFunctionCommand_h */
