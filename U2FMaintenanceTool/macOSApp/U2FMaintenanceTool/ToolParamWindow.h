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

#pragma mark for CertreqParamWindow
    - (void)prepareCertreqParamWindow;
    - (void)certreqParamWindowWillSetup:(id)sender;

@end

@protocol ToolParamWindowDelegate <NSObject>

    - (void)certreqParamWindowDidSetup:(id)sender;

@end

#endif /* ToolParamWindow_h */
