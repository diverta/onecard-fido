//
//  ToolCTAP2HealthCheckCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/02.
//
#ifndef ToolCTAP2HealthCheckCommand_h
#define ToolCTAP2HealthCheckCommand_h

#import "ToolCommon.h"
#import "ToolHIDCommand.h"

@interface ToolCTAP2HealthCheckCommand : NSObject

    - (void)pinCodeParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow
                           toolCommand:(ToolHIDCommand *)toolCommand;

    @property (nonatomic) NSString *pinCur;

@end

#endif /* ToolCTAP2HealthCheckCommand_h */
