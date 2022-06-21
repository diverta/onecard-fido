//
//  FIDOAttestationWindow.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/14.
//
#import <Cocoa/Cocoa.h>
#import "AppDefine.h"

@interface FIDOAttestationWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref;
    - (void)setCommandRef:(id)ref;
    - (Command)commandToPerform;
    - (NSArray<NSString *> *)selectedFilePaths;

@end
