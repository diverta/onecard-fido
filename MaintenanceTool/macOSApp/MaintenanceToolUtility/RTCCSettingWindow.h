//
//  RTCCSettingWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/18.
//
#ifndef RTCCSettingWindow_h
#define RTCCSettingWindow_h

#import <Cocoa/Cocoa.h>

@interface RTCCSettingWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef;

@end

#endif /* RTCCSettingWindow_h */
