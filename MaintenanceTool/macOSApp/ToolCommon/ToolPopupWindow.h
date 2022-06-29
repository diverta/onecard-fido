//
//  ToolPopupWindow.h
//  ToolCommon
//
//  Created by Makoto Morita on 2022/06/14.
//
#ifndef ToolPopupWindow_h
#define ToolPopupWindow_h

#import <Foundation/Foundation.h>

@interface ToolPopupWindow : NSObject

    + (ToolPopupWindow *)defaultWindow;
    - (bool)isButtonNoClicked;

    - (void)critical:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector
        parentWindow:(NSWindow *)parentWindow;
    - (void)criticalPrompt:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector
              parentWindow:(NSWindow *)parentWindow;
    - (void)informational:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector
             parentWindow:(NSWindow *)parentWindow;
    - (void)informationalPrompt:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector
                   parentWindow:(NSWindow *)parentWindow;

@end

#endif /* ToolPopupWindow_h */
