//
//  ToolBLEDFUCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/19.
//
#ifndef ToolBLEDFUCommand_h
#define ToolBLEDFUCommand_h

#import "AppDefine.h"

@protocol ToolBLEDFUCommandDelegate;

@interface ToolBLEDFUCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)bleDfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)bleDfuProcessingWindowNotifyCancel;

@end

@protocol ToolBLEDFUCommandDelegate <NSObject>

    - (void)notifyCommandStarted:(Command)command;
    - (void)notifyMessage:(NSString *)message;
    - (void)notifyCommandTerminated:(Command)command success:(bool)success message:(NSString *)message;

@end

#endif /* ToolBLEDFUCommand_h */
