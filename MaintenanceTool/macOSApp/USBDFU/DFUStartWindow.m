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

    @property (nonatomic, weak) ToolUSBDFUCommand *toolDFUCommand;
    @property (assign) IBOutlet NSButton        *buttonOK;
    @property (assign) IBOutlet NSButton        *buttonCancel;
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

- (void)setWindowParameter:(ToolUSBDFUCommand *)command
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
            [self enableButtons:false];
            [[self toolDFUCommand] commandWillChangeToBootloaderMode];
        } else {
            // エラーメッセージをポップアップ表示
            [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_PROMPT_USB_PORT_SET informativeText:nil withObject:nil forSelector:nil];
        }
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
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

#pragma mark - Interface for ToolDFUCommand

    - (void)commandDidChangeToBootloaderMode:(bool)success errorMessage:(NSString *)errorMessage
                                 informative:(NSString *)informative {
        if (success == false) {
            // ブートローダーモード遷移処理がNGの場合、エラーメッセージをポップアップ表示
            [[ToolPopupWindow defaultWindow] critical:errorMessage informativeText:informative withObject:self forSelector:@selector(displayErrorMessageDone)];
            return;
        }
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK];
    }

    - (void)displayErrorMessageDone {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseCancel];
    }

@end
