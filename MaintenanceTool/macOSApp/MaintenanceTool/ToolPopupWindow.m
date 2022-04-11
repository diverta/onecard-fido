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
    // ポップアップでクリックされたボタンの種類を保持
    @property (nonatomic) NSModalResponse        modalResponse;

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

    - (NSModalResponse)modalResponseOfWindow {
        return [self modalResponse];
    }

    - (void)critical:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector {
        [self windowWillOpenWithStyle:NSAlertStyleCritical messageText:message informativeText:subMessage withObject:object forSelector:selector isPrompt:false];
    }

    - (void)criticalPrompt:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector {
        [self windowWillOpenWithStyle:NSAlertStyleCritical messageText:message informativeText:subMessage withObject:object forSelector:selector isPrompt:true];
    }

    - (void)informational:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector {
        [self windowWillOpenWithStyle:NSAlertStyleInformational messageText:message informativeText:subMessage withObject:object forSelector:selector isPrompt:false];
    }

    - (void)informationalPrompt:(NSString *)message informativeText:(NSString *)subMessage withObject:(id)object forSelector:(SEL)selector {
        [self windowWillOpenWithStyle:NSAlertStyleInformational messageText:message informativeText:subMessage withObject:object forSelector:selector isPrompt:true];
    }

    - (void)windowWillOpenWithStyle:(NSAlertStyle)style messageText:(NSString *)message informativeText:(NSString *)subMessage
                         withObject:(id)object forSelector:(SEL)selector isPrompt:(bool)prompt {
        // ダイアログを作成して表示
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setAlertStyle:style];
        [alert setMessageText:message];
        if (subMessage) {
            [alert setInformativeText:subMessage];
        }
        if (prompt) {
            // Noボタンをデフォルトとする
            [alert addButtonWithTitle:@"No"];
            [alert addButtonWithTitle:@"Yes"];
        }
        ToolPopupWindow * __weak weakSelf = self;
        [alert beginSheetModalForWindow:[self parentWindow] completionHandler:^(NSModalResponse response){
            [weakSelf setModalResponse:response];
            if (object == nil || selector == nil) {
                return;
            }
            [object performSelector:selector withObject:nil afterDelay:0.0];
        }];
    }

@end
