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
    - (NSData *)generateGetKeyAgreementRequest;
    - (NSData *)generateClientPinTokenGetRequestWith:(NSData *)keyAgreementResponse;
    - (NSData *)generateMakeCredentialRequestWith:(NSData *)getPinTokenResponse;
    - (bool)parseMakeCredentialResponseWith:(NSData *)makeCredentialResponse;
    - (NSData *)generateGetAssertionRequestWith:(NSData *)getPinTokenResponse userPresence:(bool)up;
    - (bool)parseGetAssertionResponseWith:(NSData *)getAssertionResponse verifySalt:(bool)verifySalt;
    - (bool)checkStatusCode:(NSData *)responseMessage;

    // Request & Response process
    - (bool)doCTAP2Response:(Command)command responseMessage:(NSData *)response;
    - (bool)doGetKeyAgreementCommandRequest:(Command)command;

    @property (nonatomic) NSString *pinCur;

@end

#endif /* ToolCTAP2HealthCheckCommand_h */
