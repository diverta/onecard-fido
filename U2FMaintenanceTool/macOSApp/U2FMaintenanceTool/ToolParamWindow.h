//
//  ToolParamWindow.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/22.
//
#ifndef ToolParamWindow_h
#define ToolParamWindow_h

#import <Foundation/Foundation.h>

#import "ToolParameters.h"

@protocol ToolParamWindowDelegate;

    @interface ToolParamWindow : NSObject

    @property (nonatomic, weak) id<ToolParamWindowDelegate> delegate;

    - (id)initWithDelegate:(id<ToolParamWindowDelegate>)delegate;

    @property (nonatomic) KeyPairParameter  *keyPairParameter;
    @property (nonatomic) CertReqParameter  *certReqParameter;
    @property (nonatomic) SelfCertParameter *selfCertParameter;

    - (void)keypairParamWindowWillSetup:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)certreqParamWindowWillSetup:(id)sender parentWindow:(NSWindow *)parentWindow;
    - (void)selfcrtParamWindowWillSetup:(id)sender parentWindow:(NSWindow *)parentWindow;

@end

@protocol ToolParamWindowDelegate <NSObject>

    - (void)keypairParamWindowDidSetup:(id)sender;
    - (void)certreqParamWindowDidSetup:(id)sender;
    - (void)selfcrtParamWindowDidSetup:(id)sender;

@end

#endif /* ToolParamWindow_h */
