//
//  ToolBLESMPCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/26.
//
#ifndef ToolBLESMPCommand_h
#define ToolBLESMPCommand_h

#import "ToolCommon.h"

@protocol ToolBLESMPCommandDelegate;

@interface ToolBLESMPCommand : NSObject

    @property (nonatomic, weak) id<ToolBLESMPCommandDelegate> delegate;

    - (id)initWithDelegate:(id<ToolBLESMPCommandDelegate>)delegate;
    - (void)commandWillProcess:(Command)command request:(NSData *)request forCommand:(id)commandRef;

@end

@protocol ToolBLESMPCommandDelegate <NSObject>

    - (void)bleSmpCommandDidProcess:(Command)command success:(bool)success response:(NSData *)response forCommand:(id)commandRef;

@end

#endif /* ToolBLESMPCommand_h */
