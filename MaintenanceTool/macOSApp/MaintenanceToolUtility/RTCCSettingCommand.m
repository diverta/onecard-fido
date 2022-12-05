//
//  RTCCSettingCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/12/05.
//
#import "AppBLECommand.h"
#import "AppHIDCommand.h"
#import "RTCCSettingCommand.h"

@interface RTCCSettingCommand () <AppHIDCommandDelegate, AppBLECommandDelegate>

    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;
    @property (nonatomic) AppBLECommand                *appBLECommand;

@end

@implementation RTCCSettingCommand

    - (id)init {
        self = [super init];
        if (self) {
            // ヘルパークラスのインスタンスを生成
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
            [self setAppBLECommand:[[AppBLECommand alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self appHIDCommand] checkUSBHIDConnection];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
    }

#pragma mark - Call back from AppBLECommand

    - (void)didResponseCommand:(Command)command response:(NSData *)response {
    }

    - (void)didCompleteCommand:(Command)command success:(bool)success errorMessage:(NSString *)errorMessage {
    }

@end
