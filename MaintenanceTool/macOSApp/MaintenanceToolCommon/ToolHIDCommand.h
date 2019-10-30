//
//  ToolHIDCommand.h
//  MaintenanceTool
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
    - (void)displayMessage:(NSString *)string;
    - (void)hidHelperWillProcess:(Command)command;
    - (void)hidHelperWillProcess:(Command)command withData:(NSData *)data;
    - (void)setInstallParameter:(Command)command
                   skeyFilePath:(NSString *)skeyFilePath certFilePath:(NSString *)certFilePath;
    - (bool)checkUSBHIDConnection;
    - (void)doRequest:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd;
    - (void)doClientPinSetOrChange:(NSData *)message CID:(NSData *)cid;
    - (void)commandDidProcess:(Command)command result:(bool)result message:(NSString *)message;

    - (void)setPinParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)setPinParamWindowDidClose;
    - (void)pinCodeParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)pinCodeParamWindowDidClose;

@end

@protocol ToolHIDCommandDelegate <NSObject>

    - (void)notifyToolCommandMessage:(NSString *)message;
    - (void)hidCommandStartedProcess:(Command)command;
    - (void)hidCommandDidProcess:(Command)command CMD:(uint8_t)cmd response:(NSData *)resp result:(bool)result message:(NSString *)message;

@end

#endif /* ToolHIDCommand_h */
