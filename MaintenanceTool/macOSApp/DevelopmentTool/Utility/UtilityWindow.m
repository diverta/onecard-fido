//
//  UtilityWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/06/07.
//
#import "UtilityCommand.h"
#import "UtilityWindow.h"

@interface UtilityWindow ()

    // 画面項目
    @property (assign) IBOutlet NSButton                   *buttonToolVersionInfo;
    @property (assign) IBOutlet NSButton                   *buttonViewLogFile;
    @property (assign) IBOutlet NSButton                   *buttonCancel;
    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) UtilityCommand             *utilityCommand;

@end

@implementation UtilityWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
    }

    - (void)setParentWindowRef:(id)ref {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
    }

    - (void)setCommandRef:(id)ref {
        // コマンドクラスの参照を保持
        [self setUtilityCommand:(UtilityCommand *)ref];
    }

    - (IBAction)buttonToolVersionInfoDidPress:(id)sender {
    }

    - (IBAction)buttonViewLogFileDidPress:(id)sender {
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
