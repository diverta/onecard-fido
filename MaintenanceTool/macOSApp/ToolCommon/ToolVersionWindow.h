//
//  ToolVersionWindow.h
//  ToolCommon
//
//  Created by Makoto Morita on 2022/06/07.
//
#ifndef ToolVersionWindow_h
#define ToolVersionWindow_h

#import <Cocoa/Cocoa.h>

@interface ToolVersionWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref;
    - (void)setVersionInfoWithToolName:(NSString *)toolName toolVersion:(NSString *)version toolCopyright:(NSString *)copyright;

@end

#endif /* ToolVersionWindow_h */
