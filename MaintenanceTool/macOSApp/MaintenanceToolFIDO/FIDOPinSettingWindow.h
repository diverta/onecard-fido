//
//  FIDOPinSettingWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/19.
//
#ifndef FIDOPinSettingWindow_h
#define FIDOPinSettingWindow_h

#import <Cocoa/Cocoa.h>

@interface FIDOPinSettingWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef;

@end

#endif /* FIDOPinSettingWindow_h */
