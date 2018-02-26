//
//  ToolParamWindow.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/22.
//
#ifndef ToolParamWindow_h
#define ToolParamWindow_h

#import <Foundation/Foundation.h>

@protocol ToolParamWindowDelegate;

    @interface ToolParamWindow : NSObject

    @property (nonatomic, weak) id<ToolParamWindowDelegate> delegate;

    - (id)initWithDelegate:(id<ToolParamWindowDelegate>)delegate;

#pragma mark for certreq parameters
    @property (nonatomic) NSString *certreqParamPemPath;
    @property (nonatomic) NSString *certreqParamCN;
    @property (nonatomic) NSString *certreqParamOU;
    @property (nonatomic) NSString *certreqParamO;
    @property (nonatomic) NSString *certreqParamL;
    @property (nonatomic) NSString *certreqParamST;
    @property (nonatomic) NSString *certreqParamC;
    @property (nonatomic) NSString *certreqParamOutPath;

    - (void)certreqParamWindowWillSetup:(id)sender parentWindow:(NSWindow *)parentWindow;

@end

@protocol ToolParamWindowDelegate <NSObject>

    - (void)certreqParamWindowDidSetup:(id)sender;

@end

#endif /* ToolParamWindow_h */
