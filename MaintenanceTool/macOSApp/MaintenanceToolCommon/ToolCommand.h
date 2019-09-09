//
//  ToolCommand.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2017/11/20.
//
#ifndef ToolCommand_h
#define ToolCommand_h

#import "ToolCommon.h"

@protocol ToolCommandDelegate;

    @interface ToolCommand : NSObject

    @property (nonatomic, weak) id<ToolCommandDelegate> delegate;

    - (id)initWithDelegate:(id<ToolCommandDelegate>)delegate;
    - (void)toolCommandWillCreateBleRequest:(Command)command;

@end

@protocol ToolCommandDelegate <NSObject>

    - (void)notifyToolCommandMessage:(NSString *)message;
    - (void)toolCommandDidProcess:(Command)command result:(bool)result message:(NSString *)message;

@end

#endif /* ToolCommand_h */
