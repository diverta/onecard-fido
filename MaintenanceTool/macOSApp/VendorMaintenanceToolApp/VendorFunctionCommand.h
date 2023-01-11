//
//  VendorFunctionCommand.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2023/01/11.
//
#ifndef VendorFunctionCommand_h
#define VendorFunctionCommand_h

@interface VendorFunctionCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)vendorFunctionWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;

@end

#endif /* VendorFunctionCommand_h */
