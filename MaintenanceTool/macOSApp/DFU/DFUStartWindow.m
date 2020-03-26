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

    @property (nonatomic, weak) ToolDFUCommand  *toolDFUCommand;
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

- (void)setWindowParameter:(ToolDFUCommand *)command
            currentVersion:(NSString *)current
             updateVersion:(NSString *)update{
        // DFU処理クラスの参照を設定
        [self setToolDFUCommand:command];
        // バージョンラベルを設定
        [[self labelUpdateVersion] setStringValue:update];
        [[self labelCurrentVersion] setStringValue:current];
    }

    - (IBAction)buttonOKDidPress:(id)sender {
        if ([[self toolDFUCommand] checkUSBHIDConnection]) {
            // HID接続がある場合は、DFU対象デバイスをブートローダーモードに遷移させる
            [[self toolDFUCommand] commandWillChangeToBootloaderMode];
        }
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // 画面項目を初期化し、この画面を閉じる
        [self initFieldValue];
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

#pragma mark - Interface for ToolDFUCommand

    - (void)commandDidChangeToBootloaderMode:(bool)available {
        if (available == false) {
            // ブートローダーモード遷移処理がNGの場合、エラーメッセージをポップアップ表示
            [ToolPopupWindow critical:MSG_DFU_TARGET_NOT_CONNECTED
                      informativeText:nil];
            [self terminateWindow:NSModalResponseCancel];
            return;
        }
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK];
    }

@end
