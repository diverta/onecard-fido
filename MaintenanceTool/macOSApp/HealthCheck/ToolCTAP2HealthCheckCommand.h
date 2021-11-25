//
//  ToolCTAP2HealthCheckCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/02.
//
#ifndef ToolCTAP2HealthCheckCommand_h
#define ToolCTAP2HealthCheckCommand_h

#import "ToolCommon.h"
#import "ToolBLECommand.h"
#import "ToolHIDCommand.h"

@interface ToolCTAP2HealthCheckCommand : NSObject

    - (void)setTransportParam:(TransportType)type
               toolBLECommand:(ToolBLECommand *)ble toolHIDCommand:(ToolHIDCommand *)hid;
    - (void)pinCodeParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (bool)checkStatusCode:(NSData *)responseMessage;

    // Request & Response process
    - (void)doCTAP2Response:(Command)command responseMessage:(NSData *)response;
    - (void)doCTAP2Request:(Command)command;

    @property (nonatomic) NSString *pinCur;
    @property (nonatomic) NSData   *CID;

@end

#endif /* ToolCTAP2HealthCheckCommand_h */
