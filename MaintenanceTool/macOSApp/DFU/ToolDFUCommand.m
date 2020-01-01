//
//  ToolDFUCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/12/31.
//
#import <Foundation/Foundation.h>
#import <IOKit/serial/IOSerialKeys.h>

#import "ToolCDCHelper.h"
#import "ToolDFUCommand.h"

@interface ToolDFUCommand ()

    @property (nonatomic) ToolCDCHelper *toolCDCHelper;

@end

@implementation ToolDFUCommand

    - (id)init {
        self = [super init];
        
        // ToolCDCHelperのインスタンスを生成
        [self setToolCDCHelper:[[ToolCDCHelper alloc] init]];
        return self;
    }

    - (NSString *)testMain {
        return [self getConnectedDevicePath];
    }

    - (NSString *)getConnectedDevicePath {
        // 接続されているUSB CDC ACMデバイスのパスを走査
        NSArray *ACMDevicePathList = [[self toolCDCHelper] createACMDevicePathList];
        if (ACMDevicePathList != nil) {
            for (NSString *ACMDevicePath in ACMDevicePathList) {
                // 複数件存在する場合は最初に検索されたデバイスのパスを戻す
                return ACMDevicePath;
            }
        }
        // デバイスが未接続の場合は、NULLを戻す
        return nil;
    }

@end
