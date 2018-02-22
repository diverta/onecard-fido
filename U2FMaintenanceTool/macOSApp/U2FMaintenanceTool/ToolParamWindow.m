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

    - (void)prepareCertreqParamWindow {
    }

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
                NSLog(@"ToolParamWindow: CertreqParamWindow canceled");
                return;
            }
            // ファイルが選択された時の処理
            NSLog(@"ToolParamWindow: CertreqParamWindow OK");
            [weakSelf certreqParamWindowDidSetup:sender];
        }];
    }

    - (void)certreqParamWindowDidSetup:(id)sender {
        // ダイアログで入力されたパラメーターを引き渡す
        [[self delegate] certreqParamWindowDidSetup:sender];
    }

@end
