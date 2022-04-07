//
//  ToolPopupWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/02/26.
//
#import <Foundation/Foundation.h>

#import "ToolPopupWindow.h"

// このウィンドウクラスのインスタンスを保持
static ToolPopupWindow *sharedInstance;

@interface ToolPopupWindow ()

    // アプリケーションウィンドウの参照を保持
    @property (nonatomic, weak) NSWindow        *parentWindow;

@end

@implementation ToolPopupWindow

#pragma mark - Methods for singleton

    + (ToolPopupWindow *)defaultWindow {
        // このクラスのインスタンス化を１度だけ行う
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            sharedInstance = [[self alloc] init];
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

    - (void)setApplicationWindow:(NSWindow *)window {
        // アプリケーションウィンドウの参照を保持
        [self setParentWindow:window];
    }

#pragma mark - Static methods

    + (void)critical:(NSString *)message informativeText:(NSString *)subMessage {
        if (!message) {
            return;
        }
        // ダイアログを作成して表示
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setAlertStyle:NSAlertStyleCritical];
        [alert setMessageText:message];
        if (subMessage) {
            [alert setInformativeText:subMessage];
        }
        [alert runModal];
    }

    + (void)warning:(NSString *)message informativeText:(NSString *)subMessage {
        if (!message) {
            return;
        }
        // ダイアログを作成して表示
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setAlertStyle:NSAlertStyleWarning];
        [alert setMessageText:message];
        if (subMessage) {
            [alert setInformativeText:subMessage];
        }
        [alert runModal];
    }

    + (void)informational:(NSString *)message informativeText:(NSString *)subMessage {
        if (!message) {
            return;
        }
        // ダイアログを作成して表示
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setAlertStyle:NSAlertStyleInformational];
        [alert setMessageText:message];
        if (subMessage) {
            [alert setInformativeText:subMessage];
        }
        [alert runModal];
    }

    + (bool)promptYesNo:(NSString *)message informativeText:(NSString *)subMessage {
        if (!message) {
            return false;
        }
        // ダイアログを作成
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setAlertStyle:NSAlertStyleInformational];
        [alert setMessageText:message];
        if (subMessage) {
            [alert setInformativeText:subMessage];
        }
        [alert addButtonWithTitle:@"Yes"];
        [alert addButtonWithTitle:@"No"];
        // ダイアログを表示しYesボタンクリックを判定
        return ([alert runModal] == NSAlertFirstButtonReturn);
    }

@end
