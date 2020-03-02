//
//  DFUStartWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/02/24.
//
#ifndef DFUStartWindow_h
#define DFUStartWindow_h

#import <Cocoa/Cocoa.h>
#import "ToolDFUCommand.h"

@interface DFUStartWindow : NSWindowController

    @property (nonatomic) NSWindow *parentWindow;

    - (void)setWindowParameter:(ToolDFUCommand *)command
                currentVersion:(NSString *)current
                 updateVersion:(NSString *)update;
    - (void)commandDidVerifyDFUConnection:(bool)available;

@end

#endif /* DFUStartWindow_h */
