//
//  BLESettingWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/19.
//
#ifndef BLESettingWindow_h
#define BLESettingWindow_h

#import <Cocoa/Cocoa.h>

@interface BLESettingWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef;

@end

#endif /* BLESettingWindow_h */
