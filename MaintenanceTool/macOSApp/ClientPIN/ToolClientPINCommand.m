//
//  ToolClientPINCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/02.
//
#import <Foundation/Foundation.h>

#import "ToolCommonMessage.h"
#import "ToolClientPINCommand.h"

@interface ToolClientPINCommand ()

@end

@implementation ToolClientPINCommand

    - (id)init {
        self = [super init];
        NSLog(@"ToolClientPINCommand initialized");
        return self;
    }

#pragma mark - Command functions

    - (NSData *)generateSetPinMessage:(Command)command {
        NSLog(@"Set PIN sample start");
        return [[NSData alloc] init];
    }

@end
