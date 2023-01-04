//
//  AppBLECommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/14.
//
#ifndef AppBLECommand_h
#define AppBLECommand_h

#import <Foundation/Foundation.h>
#import "AppDefine.h"

@protocol AppBLECommandDelegate;

@interface AppBLECommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)doRequestCommand:(Command)command withCMD:(uint8_t)cmd withData:(NSData *)data;
    - (void)doConnectCommand;
    - (void)commandDidProcess:(bool)result message:(NSString *)message;
    - (NSString *)nameOfScannedPeripheral;

@end

@protocol AppBLECommandDelegate <NSObject>

    - (void)didResponseCommand:(Command)command response:(NSData *)response;
    - (void)didCompleteCommand:(Command)command success:(bool)success errorMessage:(NSString *)errorMessage;

@end

#endif /* AppBLECommand_h */
