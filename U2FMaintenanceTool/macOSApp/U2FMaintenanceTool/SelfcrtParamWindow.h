//
//  SelfcrtParamWindow.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/27.
//
#import <Cocoa/Cocoa.h>

#import "ToolParameters.h"

@interface SelfcrtParamWindow : NSWindowController

    @property (nonatomic) NSWindow          *parentWindow;
    @property (nonatomic) SelfCertParameter *parameter;

@end
