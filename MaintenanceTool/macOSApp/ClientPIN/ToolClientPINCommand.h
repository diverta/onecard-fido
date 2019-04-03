//
//  ToolClientPINCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/02.
//
#ifndef ToolClientPINCommand_h
#define ToolClientPINCommand_h

#import "ToolCommon.h"

@interface ToolClientPINCommand : NSObject

    - (NSData *)generateSetPinMessage:(Command)command;

    @property (nonatomic) NSString *lastErrorMessage;

@end

#endif /* ToolClientPINCommand_h */
