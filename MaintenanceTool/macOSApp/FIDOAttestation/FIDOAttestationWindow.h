//
//  FIDOAttestationWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/11/17.
//
#import <Cocoa/Cocoa.h>
#import "ToolCommon.h"

@interface FIDOAttestationWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref;
    - (void)setCommandRef:(id)ref;
    - (Command)commandToPerform;
    - (NSArray<NSString *> *)selectedFilePaths;

@end
