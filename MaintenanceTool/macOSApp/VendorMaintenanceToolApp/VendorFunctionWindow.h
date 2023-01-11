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

    - (void)setParentWindowRef:(id)ref;
    - (void)setCommandRef:(id)ref;
    - (Command)commandToPerform;

@end

#endif /* VendorFunctionWindow_h */
