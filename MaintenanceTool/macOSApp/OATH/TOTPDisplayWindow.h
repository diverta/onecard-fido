//
//  TOTPDisplayWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/22.
//
#ifndef TOTPDisplayWindow_h
#define TOTPDisplayWindow_h

#import <Cocoa/Cocoa.h>

@interface TOTPDisplayWindow : NSWindowController

    - (bool)windowWillOpenWithParentWindow:(NSWindow *)parent;

@end

#endif /* TOTPDisplayWindow_h */
