//
//  ToolClientPINCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/02.
//
#ifndef ToolClientPINCommand_h
#define ToolClientPINCommand_h

#import "ToolCommon.h"
#import "ToolHIDCommand.h"

@interface ToolClientPINCommand : NSObject

    - (NSData *)generateSetPinMessage:(Command)command;
    - (void)setPinParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow
                          toolCommand:(ToolHIDCommand *)toolCommand;

    @property (nonatomic) NSString *pinNew;
    @property (nonatomic) NSString *pinOld;

@end

#endif /* ToolClientPINCommand_h */
