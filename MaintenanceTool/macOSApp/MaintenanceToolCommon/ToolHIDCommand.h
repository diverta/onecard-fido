//
//  ToolHIDCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/03/20.
//
#ifndef ToolHIDCommand_h
#define ToolHIDCommand_h

#import "AppDefine.h"

@protocol ToolHIDCommandDelegate;

@interface ToolHIDCommand : NSObject

    @property (nonatomic, weak) id<ToolHIDCommandDelegate> delegate;

    - (id)initWithDelegate:(id<ToolHIDCommandDelegate>)delegate;
    - (void)displayMessage:(NSString *)string;
    - (void)hidHelperWillProcess:(Command)command;
    - (void)hidHelperWillProcess:(Command)command withData:(NSData *)data forCommand:(id)commandRef;
    - (void)hidHelperWillDetectConnect:(Command)command forCommand:(id)commandRef;
    - (void)hidHelperWillDetectRemoval:(Command)command forCommand:(id)commandRef;
    - (bool)checkUSBHIDConnection;
    - (void)doRequest:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd;
    - (void)doClientPinSetOrChange:(NSData *)message CID:(NSData *)cid;
    - (void)commandDidProcess:(Command)command result:(bool)result message:(NSString *)message;

    - (void)setPinParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)setPinParamWindowDidClose;

@end

@protocol ToolHIDCommandDelegate <NSObject>

    - (void)notifyToolCommandMessage:(NSString *)message;
    - (void)hidCommandStartedProcess:(Command)command;
    - (void)hidCommandDidProcess:(Command)command toolCommandRef:(id)ref CMD:(uint8_t)cmd response:(NSData *)response;
    - (void)hidCommandDidProcess:(Command)command result:(bool)result message:(NSString *)message;
    - (void)hidCommandDidDetectConnect:(Command)command forCommandRef:(id)ref;
    - (void)hidCommandDidDetectRemoval:(Command)command forCommandRef:(id)ref;

@end

#endif /* ToolHIDCommand_h */
