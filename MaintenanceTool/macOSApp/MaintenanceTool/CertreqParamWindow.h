//
//  CertreqParamWindow.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/20.
//
#import <Cocoa/Cocoa.h>

#import "ToolParameters.h"

@interface CertreqParamWindow : NSWindowController

    @property (nonatomic) NSWindow         *parentWindow;
    @property (nonatomic) CertReqParameter *parameter;

@end
