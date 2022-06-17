//
//  ToolCommonFunc.h
//  ToolCommon
//
//  Created by Makoto Morita on 2022/06/08.
//
#import <Foundation/Foundation.h>

#ifndef ToolCommonFunc_h
#define ToolCommonFunc_h

@interface ToolCommonFunc : NSObject

    + (NSString *)getAppVersionString;
    + (bool) checkMustEntry:(NSTextField *)textField informativeText:(NSString *)informativeText onWindow:(NSWindow *)window;
    + (bool) checkFileExist:(NSTextField *)textField informativeText:(NSString *)informativeText onWindow:(NSWindow *)window;
    + (bool)checkUSBHIDConnectionOnWindow:(NSWindow *)window connected:(bool)connected;

@end

#endif /* ToolCommonFunc_h */
