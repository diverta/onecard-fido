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
    + (NSString *)getAppBundleNameString;
    + (bool)isVendorMaintenanceTool;
    + (bool) checkMustEntry:(NSTextField *)textField informativeText:(NSString *)informativeText onWindow:(NSWindow *)window;
    + (bool) checkFileExist:(NSTextField *)textField informativeText:(NSString *)informativeText onWindow:(NSWindow *)window;
    + (bool)checkFileExist:(NSTextField *)textField forPath:(NSString *)path informativeText:(NSString *)informativeText onWindow:(NSWindow *)window;
    + (bool)checkUSBHIDConnectionOnWindow:(NSWindow *)window connected:(bool)connected;

    + (NSData *)commandDataForPairingRequest;
    + (NSData *)commandDataForEraseBondingData;
    + (NSData *)commandDataForChangeToBootloaderMode;
    + (NSData *)commandDataForSystemReset;
    + (NSData *)commandDataForGetFlashStat;
    + (NSData *)commandDataForGetVersionInfo;

    + (void)startTimerWithTarget:(id)targetRef forSelector:(SEL)selectorRef withObject:(id)objectRef withTimeoutSec:(NSTimeInterval)timeoutSec;
    + (void)stopTimerWithTarget:(id)targetRef forSelector:(SEL)selectorRef withObject:(id)objectRef;

@end

#endif /* ToolCommonFunc_h */
