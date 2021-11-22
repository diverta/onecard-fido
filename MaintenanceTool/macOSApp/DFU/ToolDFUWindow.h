//
//  ToolDFUWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/11/22.
//
#ifndef ToolDFUWindow_h
#define ToolDFUWindow_h

#import <Cocoa/Cocoa.h>
#import "ToolCommon.h"

@interface ToolDFUWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref;
    - (void)setCommandRef:(id)ref;
    - (Command)commandToPerform;

@end

#endif /* ToolDFUWindow_h */
