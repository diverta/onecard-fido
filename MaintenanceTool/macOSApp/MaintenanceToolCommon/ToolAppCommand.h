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
    - (void)doCommandPairing;
    - (void)doCommandEraseSkeyCert;
    - (void)doCommandInstallSkeyCert:(NSArray<NSString *> *)filePaths;
    - (void)doCommandTestCtapHidPing:(NSWindow *)parentWindow;
    - (void)doCommandBleCtap2HealthCheck:(NSWindow *)parentWindow;
    - (void)doCommandBleU2fHealthCheck:(NSWindow *)parentWindow;
    - (void)doCommandTestBlePing;
    - (void)doCommandHidCtap2HealthCheck:(NSWindow *)parentWindow;
    - (void)doCommandHidU2fHealthCheck:(NSWindow *)parentWindow;
    - (void)doCommandEraseBond:(NSWindow *)parentWindow;
    - (void)doCommandBLMode:(NSWindow *)parentWindow;
    - (void)doCommandFirmwareResetForCommandRef:(id)ref;
    - (bool)checkUSBHIDConnection;

    - (void)setPinParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)toolDFUWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)pivParamWindowWillOpenWithParent:(NSWindow *)parent;
    - (void)dfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)dfuNewProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)bleDfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow;
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
