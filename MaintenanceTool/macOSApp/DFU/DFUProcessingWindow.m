//
//  DFUProcessingWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/02/24.
//
#import "DFUProcessingWindow.h"
#import "ToolPopupWindow.h"
#import "ToolCommon.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

@interface DFUProcessingWindow ()

    @property (assign) IBOutlet NSTextField      *labelTitle;
    @property (assign) IBOutlet NSTextField      *labelProgress;
    @property (assign) IBOutlet NSLevelIndicator *levelIndicator;

@end

@implementation DFUProcessingWindow

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
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

    - (void)commandDidStartDFUProcess {
        // 画面項目を初期化
        [self initFieldValue];
    }

    - (void)commandDidNotifyDFUProcess:(NSString *)message {
        [[self labelProgress] setStringValue:message];
    }

    - (void)commandDidTerminateDFUProcess:(bool)result {
        // DFU処理が正常終了した場合はOK、異常終了した場合はAbortを戻す
        if (result) {
            [self terminateWindow:NSModalResponseOK];
        } else {
            [self terminateWindow:NSModalResponseAbort];
        }
    }

@end
