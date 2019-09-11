//
//  ToolU2FHealthCheckCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/09/11.
//
#ifndef ToolU2FHealthCheckCommand_h
#define ToolU2FHealthCheckCommand_h

#import "ToolCommon.h"
#import "ToolBLECommand.h"
#import "ToolHIDCommand.h"

@interface ToolU2FHealthCheckCommand : NSObject

    - (void)setTransportParam:(TransportType)type
               toolBLECommand:(ToolBLECommand *)ble toolHIDCommand:(ToolHIDCommand *)hid;

    // Request & Response process
    - (void)doU2FResponse:(Command)command responseMessage:(NSData *)response;
    - (void)doU2FRequest:(Command)command;

    @property (nonatomic) NSString *pinCur;
    @property (nonatomic) NSData   *CID;

@end

#endif /* ToolU2FHealthCheckCommand_h */
