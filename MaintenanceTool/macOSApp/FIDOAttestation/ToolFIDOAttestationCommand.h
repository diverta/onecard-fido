//
//  ToolFIDOAttestationCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/11/17.
//
#ifndef ToolFIDOAttestationCommand_h
#define ToolFIDOAttestationCommand_h

@interface ToolFIDOAttestationCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)fidoAttestationWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (bool)checkUSBHIDConnection;

@end

#endif /* ToolFIDOAttestationCommand_h */
