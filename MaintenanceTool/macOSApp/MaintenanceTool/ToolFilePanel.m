//
//  ToolFilePanel.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/02/22.
//
#import <Foundation/Foundation.h>
#import "ToolFilePanel.h"

@interface ToolFilePanel ()
@end

@implementation ToolFilePanel

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolFilePanelDelegate>)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
        }
        return self;
    }

#pragma mark for NSOpenPanel

    - (void)panelWillSelectPath:(id)sender parentWindow:(NSWindow *)parentWindow
                     withPrompt:(NSString *)prompt withMessage:(NSString *)message withFileTypes:(NSArray<NSString *> *)fileTypes {
        // ファイル選択パネルの設定
        NSOpenPanel *panel = [NSOpenPanel openPanel];
        [panel setAllowsMultipleSelection:NO];
        [panel setCanChooseDirectories:NO];
        [panel setCanChooseFiles:YES];
        [panel setResolvesAliases:NO];
        // プロンプト、タイトル、ファイルタイプを設定
        [panel setPrompt:prompt];
        [panel setMessage:message];
        [panel setAllowedFileTypes:fileTypes];
        // ファイル選択パネルをモーダル表示
        ToolFilePanel * __weak weakSelf = self;
        [panel beginSheetModalForWindow:parentWindow completionHandler:^(NSInteger result) {
            // 呼出元のウィンドウを表示させないようにする
            [panel orderOut:sender];
            // ファイル選択パネルで作成されたファイルパスを引き渡す
            NSString *filePath = [[panel URL] path];
            // ファイルが選択された時の処理
            [weakSelf panelDidSelectPath:sender modalResponse:result selectedPath:filePath];
        }];
    }

    - (void)panelDidSelectPath:(id)sender modalResponse:(NSInteger)modalResponse selectedPath:(NSString *)filePath {
        // ファイル選択パネルで作成されたファイルパスを引き渡す
        [[self delegate] panelDidSelectPath:sender filePath:filePath modalResponse:modalResponse];
    }

@end
