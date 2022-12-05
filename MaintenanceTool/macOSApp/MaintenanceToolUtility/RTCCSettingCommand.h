//
//  RTCCSettingCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/12/05.
//
#ifndef RTCCSettingCommand_h
#define RTCCSettingCommand_h

#import <Foundation/Foundation.h>

@interface RTCCSettingCommand : NSObject

    - (bool)isUSBHIDConnected;

@end

#endif /* RTCCSettingCommand_h */
