//
//  CertreqParamWindow.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/20.
//
#import <Cocoa/Cocoa.h>

#import "ToolParameters.h"

@interface CertreqParamWindow : NSWindowController

    @property (assign) IBOutlet NSTextField *fieldPath;
    @property (assign) IBOutlet NSTextField *fieldCN;
    @property (assign) IBOutlet NSTextField *fieldOU;
    @property (assign) IBOutlet NSTextField *fieldO;
    @property (assign) IBOutlet NSTextField *fieldL;
    @property (assign) IBOutlet NSTextField *fieldST;
    @property (assign) IBOutlet NSTextField *fieldC;

    @property (nonatomic) NSWindow         *parentWindow;
    @property (nonatomic) CertReqParameter *parameter;

@end
