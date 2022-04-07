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

    + (void)critical:     (NSString *)message informativeText:(NSString *)subMessage;
    + (void)warning:      (NSString *)message informativeText:(NSString *)subMessage;
    + (void)informational:(NSString *)message informativeText:(NSString *)subMessage;
    + (bool)promptYesNo:  (NSString *)message informativeText:(NSString *)subMessage;

@end

#endif /* ToolPopupWindow_h */
