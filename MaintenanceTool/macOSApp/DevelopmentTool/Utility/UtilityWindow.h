//
//  UtilityWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/06/07.
//
#ifndef UtilityWindow_h
#define UtilityWindow_h

#import <Cocoa/Cocoa.h>

@interface UtilityWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref;
    - (void)setCommandRef:(id)ref;

@end

#endif /* UtilityWindow_h */
