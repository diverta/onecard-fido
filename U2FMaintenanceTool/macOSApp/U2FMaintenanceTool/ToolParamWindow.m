//
//  ToolParamWindow.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/22.
//
#import <Foundation/Foundation.h>
#import "ToolParamWindow.h"
#import "ToolFilePanel.h"
#import "ToolCommonMessage.h"
#import "CertreqParamWindow.h"
#import "SelfcrtParamWindow.h"

@interface ToolParamWindow () <ToolFilePanelDelegate>

    @property (nonatomic) ToolFilePanel      *toolFilePanel;
    @property (nonatomic) CertreqParamWindow *certreqParamWindow;
    @property (nonatomic) SelfcrtParamWindow *selfcrtParamWindow;

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
        
        // 秘密鍵ファイル生成はファイル保存パネルを使用（パラメーターはここで生成）
        [self setToolFilePanel:[[ToolFilePanel alloc] initWithDelegate:self]];
        [self setKeyPairParameter:[KeyPairParameter alloc]];

        // 使用するダイアログを生成
        [self setCertreqParamWindow:[[CertreqParamWindow alloc]
                                     initWithWindowNibName:@"CertreqParamWindow"]];
        [self setSelfcrtParamWindow:[[SelfcrtParamWindow alloc]
                                     initWithWindowNibName:@"SelfcrtParamWindow"]];
        
        return self;
    }

#pragma mark for KeypairParamWindow

    - (void)keypairParamWindowWillSetup:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ファイル保存パネルをモーダル表示（親画面＝メインウィンドウ）
        [[self toolFilePanel] prepareSavePanel:MSG_BUTTON_CREATE
                                       message:MSG_PROMPT_CREATE_PEM_PATH
                                      fileName:@"U2FPrivKey" fileTypes:@[@"pem"]];
        [[self toolFilePanel] panelWillCreatePath:sender parentWindow:parentWindow];
    }

#pragma mark - Call back from ToolFilePanel

    - (void)panelDidSelectPath:(id)sender filePath:(NSString*)filePath {
    }

    - (void)panelDidCreatePath:(id)sender filePath:(NSString*)filePath {
        // ダイアログで入力されたファイルパスを引き渡し、画面を閉じる
        [[self keyPairParameter] setOutPath:filePath];
        [[self delegate] keypairParamWindowDidSetup:sender];
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

    - (void)certreqParamWindowDidSetup:(id)sender {
        // ダイアログで入力されたパラメーターを引き渡し、画面を閉じる
        [self setCertReqParameter:[[self certreqParamWindow] parameter]];
        [[self certreqParamWindow] close];
        [[self delegate] certreqParamWindowDidSetup:sender];
    }

#pragma mark for SelfcrtParamWindow

    - (void)selfcrtParamWindowWillSetup:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ダイアログが準備されていない場合は終了
        if ([self selfcrtParamWindow] == nil) {
            NSLog(@"selfcrtParamWindowWillSetup: SelfcrtParamWindow == nil");
            return;
        }
        if ([[self selfcrtParamWindow] window] == nil) {
            return;
        }
        
        // ダイアログの親ウィンドウを保持
        [[self selfcrtParamWindow] setParentWindow:parentWindow];
        
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self selfcrtParamWindow] window];
        ToolParamWindow * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            if (response == NSModalResponseCancel) {
                return;
            }
            // ダイアログで作成ボタンが押された時
            [weakSelf selfcrtParamWindowDidSetup:sender];
        }];
    }

    - (void)selfcrtParamWindowDidSetup:(id)sender {
        // ダイアログで入力されたパラメーターを引き渡し、画面を閉じる
        [self setSelfCertParameter:[[self selfcrtParamWindow] parameter]];
        [[self selfcrtParamWindow] close];
        [[self delegate] selfcrtParamWindowDidSetup:sender];
    }

@end
