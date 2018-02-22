//
//  CertreqParamWindow.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/20.
//
#import <Cocoa/Cocoa.h>

@interface CertreqParamWindow : NSWindowController

    @property (assign) IBOutlet NSTextField *fieldPath;

    @property (nonatomic) NSWindow *parentWindow;

@end
