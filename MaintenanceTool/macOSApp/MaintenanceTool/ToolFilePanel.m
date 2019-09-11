//
//  ToolFilePanel.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/02/22.
//
#import <Foundation/Foundation.h>
#import "ToolFilePanel.h"

@interface ToolFilePanel ()

    @property (nonatomic, weak) NSOpenPanel *openPanel;
    @property (nonatomic, weak) NSSavePanel *savePanel;

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
        NSOpenPanel   * __weak weakPanel = [self openPanel];
        [[self openPanel] beginSheetModalForWindow:parentWindow completionHandler:^(NSInteger result) {
            // 呼出元のウィンドウを表示させないようにする
            [weakPanel orderOut:sender];
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

#pragma mark for NSSavePanel

    - (void)prepareSavePanel:(NSString *)prompt message:(NSString *)message
                             fileName:(NSString *)fileName fileTypes:(NSArray<NSString *> *)fileTypes {
        // ファイル保存パネルの設定
        [self setSavePanel:[NSSavePanel savePanel]];
        [[self savePanel] setCanCreateDirectories:NO];
        [[self savePanel] setShowsTagField:NO];
        // プロンプト、タイトル、ファイル名、ファイルタイプを設定
        [[self savePanel] setPrompt:prompt];
        [[self savePanel] setMessage:message];
        [[self savePanel] setNameFieldStringValue:fileName];
        [[self savePanel] setAllowedFileTypes:fileTypes];
    }

    - (void)panelWillCreatePath:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ファイル保存パネルをモーダル表示
        ToolFilePanel * __weak weakSelf  = self;
        NSSavePanel   * __weak weakPanel = [self savePanel];
        [[self savePanel] beginSheetModalForWindow:parentWindow completionHandler:^(NSInteger result) {
            // 呼出元のウィンドウを表示させないようにする
            [weakPanel orderOut:sender];
            // ファイルパスが作成された時の処理
            [weakSelf panelDidCreatePath:sender modalResponse:result];
        }];
    }

    - (void)panelDidCreatePath:(id)sender modalResponse:(NSInteger)modalResponse {
        // ファイル保存パネルで作成されたファイルパスを引き渡す
        NSString *filePath = [[[self savePanel] URL] path];
        [[self delegate] panelDidCreatePath:sender filePath:filePath
                              modalResponse:modalResponse];
    }

@end
