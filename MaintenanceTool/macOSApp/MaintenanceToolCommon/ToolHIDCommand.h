//
//  ToolHIDCommand.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2019/03/20.
//
#ifndef ToolHIDCommand_h
#define ToolHIDCommand_h

#import "ToolCommon.h"

@protocol ToolHIDCommandDelegate;

@interface ToolHIDCommand : NSObject

    @property (nonatomic, weak) id<ToolHIDCommandDelegate> delegate;

    - (id)initWithDelegate:(id<ToolHIDCommandDelegate>)delegate;
    - (void)hidHelperWillProcess:(Command)command;
    - (void)setInstallParameter:(Command)command
                   skeyFilePath:(NSString *)skeyFilePath certFilePath:(NSString *)certFilePath;
    - (bool)checkUSBHIDConnection;

@end

@protocol ToolHIDCommandDelegate <NSObject>

    - (void)notifyToolCommandMessage:(NSString *)message;
    - (void)hidCommandDidProcess:(Command)command result:(bool)result message:(NSString *)message;

@end

#endif /* ToolHIDCommand_h */
