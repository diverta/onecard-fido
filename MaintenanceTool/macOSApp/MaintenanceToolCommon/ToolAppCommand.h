//
//  ToolAppCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/14.
//
#ifndef ToolAppCommand_h
#define ToolAppCommand_h

#import <Foundation/Foundation.h>
#import "AppDefine.h"

@protocol ToolAppCommandDelegate;

@interface ToolAppCommand : NSObject

    @property (nonatomic, weak) id<ToolAppCommandDelegate> delegate;

    - (id)initWithDelegate:(id<ToolAppCommandDelegate>)delegate;
    - (void)doCommandFirmwareResetForCommandRef:(id)ref;
    - (bool)checkUSBHIDConnection;

    - (void)pivParamWindowWillOpenWithParent:(NSWindow *)parent;
    - (void)pgpParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;

    - (void)commandStartedProcess:(Command)command type:(TransportType)type;
    - (void)commandDidProcess:(Command)command result:(bool)result message:(NSString *)message;

@end

@protocol ToolAppCommandDelegate <NSObject>

    - (void)commandStartedProcess:(NSString *)processNameOfCommand;
    - (void)commandDidProcess:(bool)result message:(NSString *)message processNameOfCommand:(NSString *)name;
    - (void)disableUserInterface;
    - (void)enableUserInterface;
    - (void)notifyAppCommandMessage:(NSString *)message;

@end

#endif /* ToolAppCommand_h */
