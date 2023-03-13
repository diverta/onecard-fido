//
//  OATHWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/13.
//
#import "OATHWindow.h"

// このウィンドウクラスのインスタンスを保持
static OATHWindow *sharedInstance;

@interface OATHWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;

@end

@implementation OATHWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)setParentWindowRef:(id)ref {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

@end
