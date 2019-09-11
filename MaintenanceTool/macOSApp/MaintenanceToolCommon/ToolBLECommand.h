//
//  ToolBLECommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2017/11/20.
//
#ifndef ToolBLECommand_h
#define ToolBLECommand_h

#import "ToolCommon.h"

@protocol ToolBLECommandDelegate;

@interface ToolBLECommand : NSObject

    @property (nonatomic, weak) id<ToolBLECommandDelegate> delegate;

    - (id)initWithDelegate:(id<ToolBLECommandDelegate>)delegate;
    - (void)displayMessage:(NSString *)string;
    - (void)bleCommandWillProcess:(Command)command;
    - (void)doBLECommandRequestFrom:(NSData *)dataForCommand cmd:(uint8_t)cmd;
    - (void)commandDidProcess:(bool)result message:(NSString *)message;

    - (void)pinCodeParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)pinCodeParamWindowDidClose;

@end

@protocol ToolBLECommandDelegate <NSObject>

    - (void)notifyToolCommandMessage:(NSString *)message;
    - (void)bleCommandDidProcess:(Command)command result:(bool)result message:(NSString *)message;

@end

#endif /* ToolBLECommand_h */
