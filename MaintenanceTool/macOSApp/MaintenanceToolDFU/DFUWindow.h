//
//  DFUWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/18.
//
#ifndef DFUWindow_h
#define DFUWindow_h

#import <Cocoa/Cocoa.h>

@interface DFUWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef;

@end

#endif /* DFUWindow_h */
