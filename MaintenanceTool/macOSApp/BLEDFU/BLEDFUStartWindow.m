//
//  BLEDFUStartWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/18.
//
#import "AppDefine.h"
#import "AppCommonMessage.h"
#import "BLEDFUStartWindow.h"
#import "ToolBLEDFUCommand.h"
#import "USBDFUCommand.h"

@interface BLEDFUStartWindow ()

    @property (nonatomic, weak) ToolBLEDFUCommand   *toolBLEDFUCommand;
    @property (assign) IBOutlet NSButton            *buttonOK;
    @property (assign) IBOutlet NSButton            *buttonCancel;
    @property (assign) IBOutlet NSTextField         *labelUpdateVersion;
    @property (assign) IBOutlet NSTextField         *labelCurrentVersion;
    @property (assign) IBOutlet NSTextField         *labelDescription;

@end

@implementation BLEDFUStartWindow

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

    - (void)setWindowParameter:(id)toolBLEDFUCommandRef currentVersion:(NSString *)current updateVersion:(NSString *)update{
        // DFU処理クラスの参照を設定
        if ([toolBLEDFUCommandRef isMemberOfClass:[USBDFUCommand class]]) {
            // 画面上の表示文言を変更
            [[self labelDescription] setStringValue:MSG_DESCRIPTION_START_DFU_PROCESS];
            [self setToolBLEDFUCommand:nil];
        }
        if ([toolBLEDFUCommandRef isMemberOfClass:[ToolBLEDFUCommand class]]) {
            [self setToolBLEDFUCommand:(ToolBLEDFUCommand *)toolBLEDFUCommandRef];
        }
        // バージョンラベルを設定
        [[self labelUpdateVersion] setStringValue:update];
        [[self labelCurrentVersion] setStringValue:current];
    }

    - (IBAction)buttonOKDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK];
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
