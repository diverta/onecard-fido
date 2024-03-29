//
//  AppHIDCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/07.
//
#ifndef AppHIDCommand_h
#define AppHIDCommand_h

#import <Foundation/Foundation.h>
#import "AppDefine.h"

@protocol AppHIDCommandDelegate;

@interface AppHIDCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (bool)checkUSBHIDConnection;
    - (void)doRequestCtapHidInit;
    - (void)doRequestCommand:(Command)command withCMD:(uint8_t)cmd withData:(NSData *)data;
    - (void)doRequestCtap2Command:(Command)command withCMD:(uint8_t)cmd withData:(NSData *)data;

@end

@protocol AppHIDCommandDelegate <NSObject>

    - (void)didDetectConnect;
    - (void)didDetectRemoval;
    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage;

@end

#endif /* AppHIDCommand_h */
