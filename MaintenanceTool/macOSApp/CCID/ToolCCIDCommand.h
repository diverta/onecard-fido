//
//  ToolCCIDCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/11/20.
//
#ifndef ToolCCIDCommand_h
#define ToolCCIDCommand_h

#import "ToolCommon.h"

@interface ToolCCIDCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)ccidHelperWillProcess:(Command)command;

@end

#endif /* ToolCCIDCommand_h */
