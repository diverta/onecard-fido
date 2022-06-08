//
//  ToolCommonFunc.m
//  ToolCommon
//
//  Created by Makoto Morita on 2022/06/08.
//
#import "ToolCommonFunc.h"

@interface ToolCommonFunc ()

@end

@implementation ToolCommonFunc

    + (NSString *)getAppVersionString {
        return [[NSBundle mainBundle] infoDictionary][@"CFBundleShortVersionString"];
    }

@end
