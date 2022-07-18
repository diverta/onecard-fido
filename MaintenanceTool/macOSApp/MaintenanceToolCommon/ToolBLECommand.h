//
//  ToolBLECommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2017/11/20.
//
#ifndef ToolBLECommand_h
#define ToolBLECommand_h

#import "AppDefine.h"

@protocol ToolBLECommandDelegate;

@interface ToolBLECommand : NSObject

    @property (nonatomic, weak) id<ToolBLECommandDelegate> delegate;

    - (id)initWithDelegate:(id<ToolBLECommandDelegate>)delegate;
    - (void)displayMessage:(NSString *)string;
    - (void)bleCommandWillProcess:(Command)command;
    - (void)bleCommandWillProcess:(Command)command forCommand:(id)commandRef;
    - (void)doBLECommandRequestFrom:(NSData *)dataForCommand cmd:(uint8_t)cmd;
    - (void)commandDidProcess:(bool)result message:(NSString *)message;

@end

@protocol ToolBLECommandDelegate <NSObject>

    - (void)notifyToolCommandMessage:(NSString *)message;
    - (void)bleCommandStartedProcess:(Command)command;
    - (void)bleCommandDidProcess:(Command)command toolCommandRef:(id)ref result:(bool)result response:(NSData *)response;
    - (void)bleCommandDidProcess:(Command)command result:(bool)result message:(NSString *)message;

@end

#endif /* ToolBLECommand_h */
