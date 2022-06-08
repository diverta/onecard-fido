//
//  FIDOSettingWindow.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/08.
//
#ifndef FIDOSettingWindow_h
#define FIDOSettingWindow_h

#import <Cocoa/Cocoa.h>

@interface FIDOSettingWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref;
    - (void)setCommandRef:(id)ref;
    - (Command)commandToPerform;

@end

#endif /* FIDOSettingWindow_h */
