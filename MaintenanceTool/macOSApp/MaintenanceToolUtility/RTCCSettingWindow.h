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

    - (bool)windowWillOpenWithCommandRef:(id)ref parentWindow:(NSWindow *)parent;

@end

#endif /* RTCCSettingWindow_h */
