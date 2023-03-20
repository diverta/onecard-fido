//
//  AccountSelectWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/20.
//
#ifndef AccountSelectWindow_h
#define AccountSelectWindow_h

#import <Cocoa/Cocoa.h>

@interface AccountSelectWindow : NSWindowController

    - (bool)windowWillOpenWithParentWindow:(NSWindow *)parent withTitle:(NSString *)title withCaption:(NSString *)caption ForTarget:(id)object forSelector:(SEL)selector;

@end

#endif /* AccountSelectWindow_h */
