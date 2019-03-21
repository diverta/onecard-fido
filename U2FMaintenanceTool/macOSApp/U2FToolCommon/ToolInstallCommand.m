//
//  ToolInstallCommand.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2019/03/20.
//
#import <Foundation/Foundation.h>

#import "ToolInstallCommand.h"

@interface ToolInstallCommand ()

@end

@implementation ToolInstallCommand

    - (id)init {
        self = [super init];
        NSLog(@"ToolInstallCommand initialized");
        return self;
    }

#pragma mark - Command functions

    - (NSData *)generateEraseSkeyCertMessage:(Command)command {
        // TODO: 仮の実装です。
        return [[NSData alloc] init];
    }

    - (NSData *)generateInstallSkeyCertMessage:(Command)command
                                  skeyFilePath:(NSString *)skeyFilePath
                                  certFilePath:(NSString *)certFilePath {
        // TODO: 仮の実装です。
        return [[NSData alloc] init];
    }

@end
