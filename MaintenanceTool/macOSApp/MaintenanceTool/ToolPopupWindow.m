//
//  ToolPopupWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/02/26.
//
#import <Foundation/Foundation.h>

#import "ToolPopupWindow.h"

@interface ToolPopupWindow ()

@end

@implementation ToolPopupWindow

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
