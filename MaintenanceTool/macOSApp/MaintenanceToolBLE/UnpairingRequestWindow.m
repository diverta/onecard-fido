//
//  UnpairingRequestWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/12/16.
//
#import "UnpairingRequestWindow.h"

@interface UnpairingRequestWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                  *parentWindow;
    // 画面項目の参照を保持
    @property (assign) IBOutlet NSTextField         *labelTitle;
    @property (assign) IBOutlet NSTextField         *labelProgress;
    @property (assign) IBOutlet NSLevelIndicator    *levelIndicator;
    @property (assign) IBOutlet NSButton            *buttonCancel;

@end

@implementation UnpairingRequestWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)setParentWindowRef:(id)ref {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

@end
