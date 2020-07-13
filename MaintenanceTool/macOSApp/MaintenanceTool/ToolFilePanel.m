//
//  ToolFilePanel.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/02/22.
//
#import <Foundation/Foundation.h>
#import "ToolFilePanel.h"

@interface ToolFilePanel ()

    @property (nonatomic) NSOpenPanel *openPanel;

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

    - (void)prepareOpenPanel:(NSString *)prompt message:(NSString *)message
                   fileTypes:(NSArray<NSString *> *)fileTypes {
        // ファイル選択パネルの設定
        [self setOpenPanel:[NSOpenPanel openPanel]];
        [[self openPanel] setAllowsMultipleSelection:NO];
        [[self openPanel] setCanChooseDirectories:NO];
        [[self openPanel] setCanChooseFiles:YES];
        [[self openPanel] setResolvesAliases:NO];
        // プロンプト、タイトル、ファイルタイプを設定
        [[self openPanel] setPrompt:prompt];
        [[self openPanel] setMessage:message];
        [[self openPanel] setAllowedFileTypes:fileTypes];
    }

    - (void)panelWillSelectPath:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ファイル選択パネルをモーダル表示
        ToolFilePanel * __weak weakSelf  = self;
        [[self openPanel] beginSheetModalForWindow:parentWindow completionHandler:^(NSInteger result) {
            // 呼出元のウィンドウを表示させないようにする
            [[self openPanel] orderOut:sender];
            // ファイルが選択された時の処理
            [weakSelf panelDidSelectPath:sender modalResponse:result];
        }];
    }

    - (void)panelDidSelectPath:(id)sender modalResponse:(NSInteger)modalResponse {
        // ファイル選択パネルで作成されたファイルパスを引き渡す
        NSString *filePath = [[[self openPanel] URL] path];
        [[self delegate] panelDidSelectPath:sender filePath:filePath
                              modalResponse:modalResponse];
    }

@end
