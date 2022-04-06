//
//  ToolAppCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/14.
//
#ifndef ToolAppCommand_h
#define ToolAppCommand_h

#import <Foundation/Foundation.h>
#import "ToolCommon.h"

@protocol ToolAppCommandDelegate;

@interface ToolAppCommand : NSObject

    @property (nonatomic, weak) id<ToolAppCommandDelegate> delegate;

    - (id)initWithDelegate:(id<ToolAppCommandDelegate>)delegate;
    - (void)doCommandPairing;
    - (void)doCommandEraseSkeyCert;
    - (void)doCommandInstallSkeyCert:(NSArray<NSString *> *)filePaths;
    - (void)doCommandTestCtapHidPing;
    - (void)doCommandHidGetFlashStat;
    - (void)doCommandHidGetVersionInfo;
    - (void)doCommandTestRegister;
    - (void)doCommandTestBlePing;
    - (void)doCommandHidCtap2HealthCheck;
    - (void)doCommandHidU2fHealthCheck;
    - (void)doCommandEraseBond;
    - (void)doCommandBLMode;
    - (void)doCommandFirmwareResetForCommandRef:(id)ref;
    - (bool)checkForHIDCommand;

    - (void)fidoAttestationWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)setPinParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)toolDFUWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)pinCodeParamWindowWillOpenForHID:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)pinCodeParamWindowWillOpenForBLE:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)PreferenceWindowWillOpenWithParent:(NSWindow *)parent;
    - (void)toolPreferenceWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)dfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)dfuNewProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)bleDfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)pgpParamWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;

    - (void)toolPreferenceInquiryDidProcess:(bool)result;
    - (void)resumeHealthCheckCommand;

    - (void)commandStartedProcess:(Command)command type:(TransportType)type;
    - (void)commandDidProcess:(Command)command result:(bool)result message:(NSString *)message;

@end

@protocol ToolAppCommandDelegate <NSObject>

    - (void)commandStartedProcess:(NSString *)processNameOfCommand;
    - (void)commandDidProcess:(bool)result message:(NSString *)message processNameOfCommand:(NSString *)name;
    - (void)disableUserInterface;
    - (void)notifyAppCommandMessage:(NSString *)message;
    - (void)pinCodeParamWindowWillOpenForHID;
    - (void)promptForResumeHealthCheckCommand;

@end

#endif /* ToolAppCommand_h */
