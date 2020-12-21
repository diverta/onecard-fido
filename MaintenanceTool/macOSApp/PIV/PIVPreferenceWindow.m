//
//  PIVPreferenceWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/21.
//
#import "PIVPreferenceWindow.h"
#import "ToolPIVCommand.h"
#import "ToolPopupWindow.h"
#import "ToolCommon.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

@interface PIVPreferenceWindow ()

    @property (nonatomic, weak) NSWindow            *parentWindow;

@end

@implementation PIVPreferenceWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)initFieldValue {
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        if ([self parentWindow]) {
            [[self parentWindow] endSheet:[self window] returnCode:response];
        }
    }

    - (IBAction)buttonCloseDidPress:(id)sender {
        [self terminateWindow:NSModalResponseOK];
    }

#pragma mark - For PIVPreferenceWindow open/close

    - (bool)windowWillOpenWithCommandRef:(id)ref parentWindow:(NSWindow *)parent {
        // 親画面の参照を保持
        [self setParentWindow:parent];
        // すでにダイアログが開いている場合は終了
        if ([[[self parentWindow] sheets] count] > 0) {
            return false;
        }
        // 画面項目を初期化
        [self initFieldValue];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [self window];
        PIVPreferenceWindow * __weak weakSelf = self;
        [[self parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf windowDidCloseWithSender:ref modalResponse:response];
        }];
        return true;
    }

    - (void)windowDidCloseWithSender:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [self close];
        if ([sender isMemberOfClass:[ToolPIVCommand class]]) {
            // ToolPIVCommand経由でメイン画面に制御を戻す
            ToolPIVCommand *command = (ToolPIVCommand *)sender;
            [command commandDidClosePreferenceWindow];
        }
    }

@end
