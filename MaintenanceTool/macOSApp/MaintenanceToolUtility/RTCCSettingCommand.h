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

@interface RTCCSettingCommand : NSObject

    - (bool)isUSBHIDConnected;
    - (void)commandWillPerform:(Command)command withTransportType:(TransportType)type;

@end

#endif /* RTCCSettingCommand_h */
