//
//  PGPPreferenceWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/12/27.
//
#ifndef PGPPreferenceWindow_h
#define PGPPreferenceWindow_h

#import <Cocoa/Cocoa.h>
#import "ToolCommon.h"

@interface PGPPreferenceWindow : NSWindowController

    - (bool)windowWillOpenWithCommandRef:(id)ref parentWindow:(NSWindow *)parent;
    - (void)windowDidCloseWithSender:(id)sender modalResponse:(NSInteger)modalResponse;

    - (void)toolPGPCommandDidProcess:(Command)command withResult:(bool)result withErrorMessage:(NSString *)errorMessage;

@end

#endif /* PGPPreferenceWindow_h */
