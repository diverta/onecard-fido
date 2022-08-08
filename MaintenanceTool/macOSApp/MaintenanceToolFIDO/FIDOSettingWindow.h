//
//  FIDOSettingWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/18.
//
#ifndef FIDOSettingWindow_h
#define FIDOSettingWindow_h

#import <Cocoa/Cocoa.h>

@interface FIDOSettingWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef;

@end

#endif /* FIDOSettingWindow_h */
