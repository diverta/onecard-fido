//
//  ToolParamWindow.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/22.
//
#import <Foundation/Foundation.h>
#import "ToolParamWindow.h"
#import "CertreqParamWindow.h"

@interface ToolParamWindow ()

    @property (nonatomic) CertreqParamWindow *certreqParamWindow;

@end

@implementation ToolParamWindow

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolParamWindowDelegate>)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
        }
        
        // 使用するダイアログを生成
        [self setCertreqParamWindow:[[CertreqParamWindow alloc]
                                     initWithWindowNibName:@"CertreqParamWindow"]];
        NSLog(@"ToolParamWindow: new CertreqParamWindow created");
        
        return self;
    }

#pragma mark for CertreqParamWindow

    - (void)certreqParamWindowWillSetup:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ダイアログが準備されていない場合は終了
        if ([self certreqParamWindow] == nil) {
            return;
        }
        if ([[self certreqParamWindow] window] == nil) {
            return;
        }
        
        // ダイアログの親ウィンドウを保持
        [[self certreqParamWindow] setParentWindow:parentWindow];
        
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self certreqParamWindow] window];
        ToolParamWindow * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            if (response == NSModalResponseCancel) {
                return;
            }
            // ダイアログで作成ボタンが押された時
            [weakSelf certreqParamWindowDidSetup:sender];
        }];
    }

    - (void)getCertreqParams {
        // 画面項目値を各プロパティーに保持
        CertreqParamWindow *window = [self certreqParamWindow];
        [self setCertreqParamPemPath:[[window fieldPath] stringValue]];
        [self setCertreqParamCN:[[window fieldCN] stringValue]];
        [self setCertreqParamOU:[[window fieldOU] stringValue]];
        [self setCertreqParamO:[[window fieldO] stringValue]];
        [self setCertreqParamL:[[window fieldL] stringValue]];
        [self setCertreqParamST:[[window fieldST] stringValue]];
        [self setCertreqParamC:[[window fieldC] stringValue]];
        [self setCertreqParamOutPath:[[self certreqParamWindow] outputPath]];
    }

    - (void)certreqParamWindowDidSetup:(id)sender {
        // ダイアログで入力されたパラメーターを引き渡す
        [self getCertreqParams];
        [[self delegate] certreqParamWindowDidSetup:sender];
    }

@end
