//
//  SetPinParamWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/02/27.
//
#import <Cocoa/Cocoa.h>
#import "ToolClientPINCommand.h"

@interface SetPinParamWindow : NSWindowController

    @property (nonatomic) NSWindow *parentWindow;
    @property (nonatomic) ToolClientPINCommand *toolClientPINCommand;

@end
