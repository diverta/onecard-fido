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
    // コマンド実行クラスの参照を保持
    @property (assign) id                            callbackTarget;
    @property (assign) SEL                           callbackSelector;

@end

@implementation UnpairingRequestWindow

    - (void)windowDidLoad {
        // 画面項目を初期化
        [super windowDidLoad];
        [self initFieldValue];
    }

    - (void)initFieldValue {
        [[self levelIndicator] setIntegerValue:0];
    }

    - (void)setParentWindowRef:(id)ref {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // Cancelボタンを使用不可とする
        [[self buttonCancel] setEnabled:false];
        // コマンドクラスの関数をコールバック
        [[self callbackTarget] performSelector:[self callbackSelector] withObject:nil afterDelay:0.0];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

    - (void)commandDidStartUnpairingRequestProcessForTarget:(id)target forSelector:(SEL)selector withProgressMax:(int)progressMax {
        // コマンドクラスの参照を保持
        [self setCallbackTarget:target];
        [self setCallbackSelector:selector];
        // 画面項目を初期化
        [self initFieldValue];
        [[self levelIndicator] setMaxValue:progressMax];
        // Cancelボタンを使用可とする
        [[self buttonCancel] setEnabled:true];
    }

    - (void)commandDidCancelUnpairingRequestProcess {
        // 処理がキャンセルされた場合はCancelを戻す
        [self terminateWindow:NSModalResponseCancel];
    }

@end
