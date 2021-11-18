//
//  ToolBLEDFUCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/19.
//
#ifndef ToolBLEDFUCommand_h
#define ToolBLEDFUCommand_h

#import "ToolCommon.h"

@interface ToolBLEDFUCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)bleDfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow toolBLECommandRef:(id)toolBLECommandRef;
    - (void)bleDfuProcessingWindowNotifyCancel;
    - (void)toolBLECommandDidProcess:(Command)command success:(bool)success response:(NSData *)response;

@end

#endif /* ToolBLEDFUCommand_h */
