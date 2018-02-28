//
//  ToolParamWindow.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/22.
//
#ifndef ToolParamWindow_h
#define ToolParamWindow_h

#import <Foundation/Foundation.h>

#import "ToolCommon.h"
#import "ToolParameters.h"

@protocol ToolParamWindowDelegate;

    @interface ToolParamWindow : NSObject

    @property (nonatomic, weak) id<ToolParamWindowDelegate> delegate;

    - (id)initWithDelegate:(id<ToolParamWindowDelegate>)delegate;

    @property (nonatomic) KeyPairParameter  *keyPairParameter;
    @property (nonatomic) CertReqParameter  *certReqParameter;
    @property (nonatomic) SelfCertParameter *selfCertParameter;

    - (void)paramWindowWillSetup:(id)sender parentWindow:(NSWindow *)parentWindow command:(Command)command;

#pragma mark - Utilities for check entry
    + (bool) checkMustEntry:(NSTextField *)textField informativeText:(NSString *)informativeText;
    + (bool) checkFileExist:(NSTextField *)textField informativeText:(NSString *)informativeText;

@end

@protocol ToolParamWindowDelegate <NSObject>

    - (void)paramWindowDidSetup:(id)sender;

@end

#endif /* ToolParamWindow_h */
