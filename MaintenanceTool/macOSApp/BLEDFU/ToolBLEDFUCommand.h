//
//  ToolBLEDFUCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/19.
//
#ifndef ToolBLEDFUCommand_h
#define ToolBLEDFUCommand_h

@interface ToolBLEDFUCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)getVersionInfoWithCommand:(ToolBLECommand *)toolBLECommand;
    - (void)toolBLECommandDidProcess:(Command)command response:(NSData *)response;

@end

#endif /* ToolBLEDFUCommand_h */
