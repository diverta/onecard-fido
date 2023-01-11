//
//  VendorFunctionWindow.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2023/01/11.
//
#ifndef VendorFunctionWindow_h
#define VendorFunctionWindow_h

#import <Cocoa/Cocoa.h>

@interface VendorFunctionWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef;
    - (void)vendorFunctionCommandDidProcess;

@end

#endif /* VendorFunctionWindow_h */
