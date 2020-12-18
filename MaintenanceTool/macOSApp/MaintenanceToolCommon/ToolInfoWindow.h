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

    @property (nonatomic, weak) NSWindow *parentWindow;

    - (bool)windowWillOpenWithCommandRef:(id)ref titleString:(NSString *)title infoString:(NSString *)info;

@end

#endif /* ToolInfoWindow_h */
