//
//  PIVPreferenceWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/21.
//
#ifndef PIVPreferenceWindow_h
#define PIVPreferenceWindow_h

#import <Cocoa/Cocoa.h>
#import "ToolCommon.h"

@interface PIVPreferenceWindow : NSWindowController

    - (bool)windowWillOpenWithCommandRef:(id)ref parentWindow:(NSWindow *)parent;
    - (void)windowDidCloseWithSender:(id)sender modalResponse:(NSInteger)modalResponse;

    - (void)toolPIVCommandDidProcess:(Command)command withResult:(bool)result withErrorMessage:(NSString *)errorMessage;

@end

#endif /* PIVPreferenceWindow_h */
