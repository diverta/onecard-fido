//
//  BLEDFUProcessingWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/21.
//
#import "AppCommonMessage.h"
#import "BLEDFUDefine.h"
#import "BLEDFUProcessingWindow.h"
#import "ToolBLEDFUCommand.h"
#import "ToolCommon.h"
#import "ToolLogFile.h"
#import "ToolPopupWindow.h"

@interface BLEDFUProcessingWindow ()

    @property (assign) IBOutlet NSTextField         *labelTitle;
    @property (assign) IBOutlet NSTextField         *labelProgress;
    @property (assign) IBOutlet NSLevelIndicator    *levelIndicator;
    @property (assign) IBOutlet NSButton            *buttonCancel;

    // コマンドクラスの参照を保持
    @property (nonatomic) ToolBLEDFUCommand         *toolBLEDFUCommand;

@end

@implementation BLEDFUProcessingWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        // 画面項目を初期化
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // テキストをブランクに設定
        [[self labelTitle] setStringValue:MSG_DFU_PROCESS_TITLE_GOING];
        [[self labelProgress] setStringValue:@""];
        [[self levelIndicator] setIntegerValue:0];
        // Cancelボタンを使用不可とする
        [[self buttonCancel] setEnabled:false];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // Cancelボタンを使用不可とする
        [[self buttonCancel] setEnabled:false];
        // Cancelボタンがクリックされた旨をコマンドクラスに通知
        [[self toolBLEDFUCommand] bleDfuProcessingWindowNotifyCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

    - (void)commandDidStartDFUProcess:(id)toolCommandRef maxProgressValue:(int)progressMax {
        // コマンドクラスの参照を保持
        [self setToolBLEDFUCommand:(ToolBLEDFUCommand *)toolCommandRef];
        // 画面項目を初期化
        [self initFieldValue];
        [[self levelIndicator] setMaxValue:progressMax];
    }

    - (void)commandDidNotifyDFUProcess:(NSString *)message progressValue:(int)progress {
        // 進捗メッセージ、プログレスバーを更新
        [[self labelProgress] setStringValue:message];
        [[self levelIndicator] setIntValue:progress];
    }

    - (void)commandDidNotifyCancelable:(bool)cancelable {
        // Cancelボタンを使用可／不可とする
        [[self buttonCancel] setEnabled:cancelable];
    }

    - (void)commandDidTerminateDFUProcess:(bool)result {
        // DFU処理が正常終了した場合はOK、異常終了した場合はAbortを戻す
        if (result) {
            [self terminateWindow:NSModalResponseOK];
        } else {
            [self terminateWindow:NSModalResponseAbort];
        }
    }

    - (void)commandDidCancelDFUProcess {
        // DFU処理がキャンセルされた場合はCancelを戻す
        [self terminateWindow:NSModalResponseCancel];
    }

@end
