//
//  ToolProcessingWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/01/24.
//
#ifndef ToolProcessingWindow_h
#define ToolProcessingWindow_h

#import <Cocoa/Cocoa.h>

@interface ToolProcessingWindow : NSWindowController

    + (ToolProcessingWindow *)defaultWindow;
    - (bool)windowWillOpenWithCommandRef:(id)ref withParentWindow:(NSWindow *)parent;
    - (void)windowWillClose:(NSModalResponse)response;

@end

#endif /* ToolProcessingWindow_h */
