//
//  ToolDFUCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/11/22.
//
#ifndef ToolDFUCommand_h
#define ToolDFUCommand_h

@interface ToolDFUCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)toolDFUWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (bool)checkUSBHIDConnection;

@end

#endif /* ToolDFUCommand_h */
