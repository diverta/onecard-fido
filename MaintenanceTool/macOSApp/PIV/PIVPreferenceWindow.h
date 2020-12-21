//
//  PIVPreferenceWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/21.
//
#ifndef PIVPreferenceWindow_h
#define PIVPreferenceWindow_h

#import <Cocoa/Cocoa.h>

@interface PIVPreferenceWindow : NSWindowController

    - (bool)windowWillOpenWithCommandRef:(id)ref parentWindow:(NSWindow *)parent;
    - (void)windowDidCloseWithSender:(id)sender modalResponse:(NSInteger)modalResponse;

@end

#endif /* PIVPreferenceWindow_h */
