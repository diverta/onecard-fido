//
//  PinCodeParamWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/02/27.
//
#import <Cocoa/Cocoa.h>
#import "ToolCTAP2HealthCheckCommand.h"

@interface PinCodeParamWindow : NSWindowController

    @property (nonatomic) NSWindow *parentWindow;
    @property (nonatomic) ToolCTAP2HealthCheckCommand *toolCTAP2HealthCheckCommand;

@end
