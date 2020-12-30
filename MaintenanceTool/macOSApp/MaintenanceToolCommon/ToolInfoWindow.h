//
//  ToolInfoWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/17.
//
#ifndef ToolInfoWindow_h
#define ToolInfoWindow_h

#import <Cocoa/Cocoa.h>

@interface ToolInfoWindow : NSWindowController

    + (ToolInfoWindow *)defaultWindow;
    - (bool)windowWillOpenWithCommandRef:(id)ref withParentWindow:(NSWindow *)parent titleString:(NSString *)title infoString:(NSString *)info;

@end

#endif /* ToolInfoWindow_h */
