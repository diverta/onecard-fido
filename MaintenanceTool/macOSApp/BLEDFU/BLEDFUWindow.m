//
//  BLEDFUWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/18.
//
#import "BLEDFUWindow.h"
#import "ToolAppCommand.h"

@interface BLEDFUWindow ()

    @property (nonatomic, weak) ToolAppCommand  *toolAppCommand;

    @property (nonatomic, weak) NSWindow        *parentWindow;
    @property (assign) IBOutlet NSButton        *buttonOK;
    @property (assign) IBOutlet NSButton        *buttonCancel;
    @property (assign) IBOutlet NSTextField     *labelUpdateVersion;
    @property (assign) IBOutlet NSTextField     *labelCurrentVersion;

@end

@implementation BLEDFUWindow

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

    - (void)setToolAppCommandRef:(id)ref {
        // コマンドクラスの参照を設定
        [self setToolAppCommand:(ToolAppCommand *)ref];
    }

    - (IBAction)buttonOKDidPress:(id)sender {
        // TODO: 仮の実装です。
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)enableButtons:(bool)enabled {
        [[self buttonOK] setEnabled:enabled];
        [[self buttonCancel] setEnabled:enabled];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // 画面項目を初期化し、この画面を閉じる
        [self initFieldValue];
        [self enableButtons:true];
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

@end
