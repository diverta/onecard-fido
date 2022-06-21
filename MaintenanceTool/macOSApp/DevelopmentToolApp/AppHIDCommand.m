//
//  AppHIDCommand.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/17.
//
#import "AppHIDCommand.h"
#import "ToolHIDHelper.h"

@interface AppHIDCommand () <ToolHIDHelperDelegate>

    @property (nonatomic) ToolHIDHelper *toolHIDHelper;

@end

@implementation AppHIDCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
            [self setToolHIDHelper:[[ToolHIDHelper alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        return [[self toolHIDHelper] isDeviceConnected];
    }

#pragma mark - Call back from ToolHIDHelper

    - (void)hidHelperDidDetectConnect {
        [[self delegate] didDetectConnect];
    }

    - (void)hidHelperDidDetectRemoval {
        [[self delegate] didDetectRemoval];
    }

    - (void)hidHelperDidReceive:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd {
    }

    - (void)hidHelperDidResponseTimeout {
    }

@end
