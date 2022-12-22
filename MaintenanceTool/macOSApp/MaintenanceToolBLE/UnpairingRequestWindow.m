//
//  UnpairingRequestWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/12/16.
//
#import "AppCommonMessage.h"
#import "UnpairingRequestWindow.h"

@interface UnpairingRequestWindow ()

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
        [[self labelTitle] setStringValue:MSG_BLE_UNPAIRING_PREPARATION];
        [[self labelProgress] setStringValue:MSG_NONE];
        [[self levelIndicator] setIntegerValue:0];
        [[self levelIndicator] setIntValue:0];
        // Cancelボタンを使用不可とする
        [[self buttonCancel] setEnabled:false];
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
        // 画面項目を初期化
        [self initFieldValue];
    }

    - (void)commandDidStartUnpairingRequestProcessForTarget:(id)target forSelector:(SEL)selector withProgressMax:(int)progressMax {
        // コマンドクラスの参照を保持
        [self setCallbackTarget:target];
        [self setCallbackSelector:selector];
        // 画面項目を初期化
        [self initFieldValue];
        [[self levelIndicator] setMaxValue:progressMax];
        [[self levelIndicator] setIntValue:progressMax];
    }

    - (void)commandDidStartWaitingForUnpairWithDeviceName:(NSString *)deviceName {
        // メッセージを表示
        NSString *message = [NSString stringWithFormat:MSG_BLE_UNPAIRING_WAIT_DISCONNECT, deviceName];
        [[self labelTitle] setStringValue:message];
        // Cancelボタンを使用可とする
        [[self buttonCancel] setEnabled:true];
    }

    - (void)commandDidNotifyProcessWithMessage:(NSString *)message withProgress:(int)progress {
        // 残り秒数が０になった場合
        if (progress == 0) {
            // Cancelボタンを使用不可とする
            [[self buttonCancel] setEnabled:false];
            // ラベルを更新
            [[self labelTitle] setStringValue:MSG_BLE_UNPAIRING_WAIT_DISC_TIMEOUT];
        }
        // 残り秒数表示ラベル、プログレスバーを更新
        [[self labelProgress] setStringValue:message];
        [[self levelIndicator] setIntValue:progress];
    }

    - (void)commandDidCancelUnpairingRequestProcess {
        // 処理がキャンセルされた場合はCancelを戻す
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)commandDidTerminateUnpairingRequestProcess:(bool)success {
        // 処理が正常終了した場合はOK、異常終了した場合はAbortを戻す
        if (success) {
            [self terminateWindow:NSModalResponseOK];
        } else {
            [self terminateWindow:NSModalResponseAbort];
        }
    }

@end
