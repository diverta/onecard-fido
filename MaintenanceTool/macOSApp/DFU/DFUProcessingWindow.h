//
//  DFUProcessingWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/02/24.
//
#ifndef DFUProcessingWindow_h
#define DFUProcessingWindow_h

#import <Cocoa/Cocoa.h>
#import "ToolDFUCommand.h"

@interface DFUProcessingWindow : NSWindowController

    @property (nonatomic) NSWindow *parentWindow;

@end

#endif /* DFUProcessingWindow_h */
