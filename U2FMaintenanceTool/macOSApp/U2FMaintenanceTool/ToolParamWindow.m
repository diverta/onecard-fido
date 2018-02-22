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
        return self;
    }

#pragma mark for CertreqParamWindow

    - (void)prepareCertreqParamWindow {
        // ダイアログ画面の参照を取得
        if ([self certreqParamWindow] == nil) {
            [self setCertreqParamWindow:[[CertreqParamWindow alloc]
                                         initWithWindowNibName:@"CertreqParamWindow"]];
            NSLog(@"ToolParamWindow: new CertreqParamWindow created");
        }
    }

    - (void)certreqParamWindowWillSetup:(id)sender {
        // ダイアログが準備されていない場合は終了
        if ([self certreqParamWindow] == nil) {
            return;
        }
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self certreqParamWindow] window];
        NSModalResponse response = [NSApp runModalForWindow:dialog];
        [NSApp endSheet:dialog];
        // 親のウィンドウを表示させないようにする
        [dialog orderOut:sender];
        // ダイアログからの戻りがCancelの場合は処理中止
        if (response == NSModalResponseCancel) {
            NSLog(@"ToolParamWindow: CertreqParamWindow canceled");
            return;
        }
        // ダイアログからの戻りがOKの場合は処理続行
        NSLog(@"ToolParamWindow: CertreqParamWindow OK");
        [[self delegate] certreqParamWindowDidSetup:sender];
    }

@end
