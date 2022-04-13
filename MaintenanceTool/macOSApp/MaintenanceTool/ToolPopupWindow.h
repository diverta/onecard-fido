//
//  ToolPopupWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/02/26.
//
#ifndef ToolPopupWindow_h
#define ToolPopupWindow_h

@interface ToolPopupWindow : NSObject

    + (ToolPopupWindow *)defaultWindow;
    - (void)setApplicationWindow:(NSWindow *)window;
    - (bool)isButtonNoClicked;

    - (void)critical:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector;
    - (void)critical:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector
        parentWindow:(NSWindow *)parentWindow;
    - (void)criticalPrompt:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector;
    - (void)criticalPrompt:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector
              parentWindow:(NSWindow *)parentWindow;
    - (void)informational:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector;
    - (void)informational:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector
             parentWindow:(NSWindow *)parentWindow;
    - (void)informationalPrompt:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector;
    - (void)informationalPrompt:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector
                   parentWindow:(NSWindow *)parentWindow;

@end

#endif /* ToolPopupWindow_h */
