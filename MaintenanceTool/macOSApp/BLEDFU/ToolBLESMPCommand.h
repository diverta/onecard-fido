//
//  ToolBLESMPCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/27.
//
#ifndef ToolBLESMPCommand_h
#define ToolBLESMPCommand_h

#import "ToolCommon.h"

@protocol ToolBLESMPCommandDelegate;

@interface ToolBLESMPCommand : NSObject

    @property (nonatomic, weak) id<ToolBLESMPCommandDelegate> delegate;

    - (id)initWithDelegate:(id<ToolBLESMPCommandDelegate>)delegate;
    - (void)commandWillConnect;
    - (void)commandWillProcess:(Command)command request:(NSData *)request forCommand:(id)commandRef;
    - (void)commandWillDisconnect;

    // 転送済みバイト数を保持
    @property (nonatomic) size_t imageBytesSent;

@end

@protocol ToolBLESMPCommandDelegate <NSObject>

    - (void)bleSmpCommandDidConnect;
    - (void)bleSmpCommandDidProcess:(Command)command success:(bool)success response:(NSData *)response forCommand:(id)commandRef;
    - (void)bleSmpCommandDidDisconnectWithError:(NSError *)error;

@end

#endif /* ToolBLESMPCommand_h */
