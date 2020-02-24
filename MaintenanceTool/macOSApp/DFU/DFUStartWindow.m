//
//  DFUStartWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/02/24.
//
#import "DFUStartWindow.h"
#import "ToolPopupWindow.h"
#import "ToolCommon.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

@interface DFUStartWindow ()

    @property (assign) IBOutlet NSTextField     *labelUpdateVersion;
    @property (assign) IBOutlet NSTextField     *labelCurrentVersion;

@end

@implementation DFUStartWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        // 画面項目を初期化
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // テキストをブランクに設定
        [[self labelUpdateVersion] setStringValue:@""];
        [[self labelCurrentVersion] setStringValue:@""];
    }

    - (void)setLabelVersion:(NSString *)current updateVersion:(NSString *)update {
        // バージョンラベルを設定
        [[self labelUpdateVersion] setStringValue:update];
        [[self labelCurrentVersion] setStringValue:current];
    }

    - (IBAction)buttonOKDidPress:(id)sender {
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // 画面項目を初期化し、この画面を閉じる
        [self initFieldValue];
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

@end
