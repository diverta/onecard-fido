//
//  ToolPGPCcidCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/03/02.
//
#ifndef ToolPGPCcidCommand_h
#define ToolPGPCcidCommand_h

#import "ToolCommon.h"

@protocol ToolPGPCcidCommandDelegate;

@interface ToolPGPCcidCommand : NSObject

    - (id)initWithDelegate:(id<ToolPGPCcidCommandDelegate>)delegate;
    - (void)ccidCommandWillProcess:(Command)command withCommandParameter:(id)parameter;

@end

@protocol ToolPGPCcidCommandDelegate <NSObject>

    - (void)ccidCommandDidNotifyErrorMessage:(NSString *)message;
    - (void)ccidCommandDidProcess:(bool)success;

@end


#endif /* ToolPGPCcidCommand_h */
