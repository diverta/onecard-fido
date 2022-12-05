//
//  RTCCSettingCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/12/05.
//
#ifndef RTCCSettingCommand_h
#define RTCCSettingCommand_h

#import <Foundation/Foundation.h>
#import "AppDefine.h"

@protocol RTCCSettingCommandDelegate;

@interface RTCCSettingCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (bool)isUSBHIDConnected;
    - (void)commandWillPerform:(Command)command withTransportType:(TransportType)type;

@end

@protocol RTCCSettingCommandDelegate <NSObject>

    - (void)RTCCSettingCommandDidProcess:(Command)command commandName:(NSString *)commandName withTimestamp:(NSArray<NSString *>*)timestamps
                              withResult:(bool)result withErrorMessage:(NSString *)errorMessage;

@end

#endif /* RTCCSettingCommand_h */
