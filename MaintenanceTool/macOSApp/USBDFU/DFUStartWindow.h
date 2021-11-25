//
//  DFUStartWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/02/24.
//
#ifndef DFUStartWindow_h
#define DFUStartWindow_h

#import <Cocoa/Cocoa.h>
#import "ToolUSBDFUCommand.h"

@interface DFUStartWindow : NSWindowController

    @property (nonatomic) NSWindow *parentWindow;

    - (void)setWindowParameter:(ToolUSBDFUCommand *)command
                currentVersion:(NSString *)current
                 updateVersion:(NSString *)update;
    - (void)commandDidChangeToBootloaderMode:(bool)success errorMessage:(NSString *)errorMessage
                                 informative:(NSString *)informative;

@end

#endif /* DFUStartWindow_h */
