//
//  ToolProcessingWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/01/24.
//
#import "ToolProcessingWindow.h"
#import "ToolPopupWindow.h"

// このウィンドウクラスのインスタンスを保持
static ToolProcessingWindow *sharedInstance;

@interface ToolProcessingWindow ()

    // 親ウィンドウの参照を保持
    @property (nonatomic, weak) NSWindow        *parentWindow;
    // 処理終了メッセージを保持
    @property (nonatomic) NSString              *messageText;
    @property (nonatomic) NSString              *informativeText;

@end

@implementation ToolProcessingWindow

#pragma mark - Methods for singleton

    + (ToolProcessingWindow *)defaultWindow {
        // このクラスのインスタンス化を１度だけ行う
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            sharedInstance = [[self alloc] initWithWindowNibName:@"ToolProcessingWindow"];
        });
        // インスタンスの参照を戻す
        return sharedInstance;
    }

    + (id)allocWithZone:(NSZone *)zone {
        // このクラスのインスタンス化を１度だけ行う
        __block id ret = nil;
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            sharedInstance = [super allocWithZone:zone];
            ret = sharedInstance;
        });
        
        // インスタンスの参照を戻す（２回目以降の呼び出しではnilが戻る）
        return ret;
    }

    - (id)copyWithZone:(NSZone *)zone{
        return self;
    }

#pragma mark - Methods of this instance

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

#pragma mark - For ToolProcessingWindow open/close

    - (bool)windowWillOpenWithCommandRef:(id)ref withParentWindow:(NSWindow *)parent {
        // すでにダイアログが開いている場合は終了
        [self setParentWindow:parent];
        if ([[[self parentWindow] sheets] count] > 0) {
            return false;
        }
        // ダイアログをモーダルで表示
        NSWindow *dialog = [self window];
        ToolProcessingWindow * __weak weakSelf = self;
        [[self parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf toolInfoWindowDidClose:ref modalResponse:response];
        }];
        return true;
    }

    - (void)toolInfoWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [self close];
        // 処理終了メッセージをポップアップ表示
        if (modalResponse == NSModalResponseOK) {
            [[ToolPopupWindow defaultWindow] informational:[self messageText] informativeText:[self informativeText] withObject:nil forSelector:nil
                                              parentWindow:[self parentWindow]];
        }
        if (modalResponse == NSModalResponseAbort) {
            [[ToolPopupWindow defaultWindow] critical:[self messageText] informativeText:[self informativeText] withObject:nil forSelector:nil
                                         parentWindow:[self parentWindow]];
        }
    }

    - (void)windowWillClose:(NSModalResponse)response withMessage:(NSString *)message withInformative:(NSString *)informative {
        // 処理終了メッセージを退避
        [self setMessageText:message];
        [self setInformativeText:informative];
        // この画面を閉じる
        if ([self parentWindow] && [[[self parentWindow] sheets] count] > 0) {
            [[self parentWindow] endSheet:[self window] returnCode:response];
        }
    }

@end
