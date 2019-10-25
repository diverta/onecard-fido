//
//  ToolPreferenceWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/10/25.
//
#ifndef ToolPreferenceWindow_h
#define ToolPreferenceWindow_h

#import <Cocoa/Cocoa.h>
#import "ToolPreferenceCommand.h"

@interface ToolPreferenceWindow : NSWindowController

    @property (nonatomic) NSWindow              *parentWindow;
    @property (nonatomic) ToolPreferenceCommand *toolPreferenceCommand;

@end

#endif /* ToolPreferenceWindow_h */
