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
    - (void)doConnect;
    - (void)doRequestCommand:(Command)command withCMD:(uint8_t)cmd withData:(NSData *)data;
    - (void)doDisconnect;

@end

@protocol AppBLECommandDelegate <NSObject>

    - (void)didConnect:(bool)success errorMessage:(NSString *)errorMessage;
    - (void)didDisconnect:(bool)success errorMessage:(NSString *)errorMessage;
    - (void)didResponseCommand:(Command)command response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage;

@end

#endif /* AppBLECommand_h */
