//
//  BLESettingCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/19.
//
#import "AppCommonMessage.h"
#import "BLEPairingCommand.h"
#import "BLESettingCommand.h"
#import "BLESettingWindow.h"
#import "BLEUnpairingCommand.h"

@implementation BLESettingCommandParameter

@end

@interface BLESettingCommand () <BLEPairingCommandDelegate, BLEUnpairingCommandDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) BLESettingWindow             *bleSettingWindow;
    // 下位クラスの参照を保持
    @property (nonatomic) BLEPairingCommand            *blePairingCommand;
    @property (nonatomic) BLEUnpairingCommand          *bleUnpairingCommand;
    // 処理のパラメーターを保持
    @property (nonatomic) BLESettingCommandParameter   *commandParameter;

@end

@implementation BLESettingCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // 画面のインスタンスを生成
            [self setBleSettingWindow:[[BLESettingWindow alloc] initWithWindowNibName:@"BLESettingWindow"]];
            // ヘルパークラスのインスタンスを生成
            [self setCommandParameter:[[BLESettingCommandParameter alloc] init]];
            [self setBlePairingCommand:[[BLEPairingCommand alloc] initWithDelegate:self]];
            [self setBleUnpairingCommand:[[BLEUnpairingCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (void)bleSettingWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self bleSettingWindow] setParentWindowRef:parentWindow withCommandRef:self withParameterRef:[self commandParameter]];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self bleSettingWindow] window];
        BLESettingCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf bleSettingWindowDidClose:self modalResponse:response];
        }];
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self blePairingCommand] isUSBHIDConnected];
    }

#pragma mark - Perform functions

    - (void)bleSettingWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self bleSettingWindow] close];
        // 実行コマンドにより処理分岐
        switch ([[self commandParameter] command]) {
            case COMMAND_PAIRING:
                [self notifyCommandStartedWithCommandName:PROCESS_NAME_PAIRING];
                [[self blePairingCommand] doRequestBlePairing];
                break;
            case COMMAND_ERASE_BONDS:
                [self notifyCommandStartedWithCommandName:PROCESS_NAME_ERASE_BONDS];
                [[self blePairingCommand] doRequestHidEraseBonds];
                break;
            case COMMAND_UNPAIRING_REQUEST:
                [self notifyCommandStartedWithCommandName:PROCESS_NAME_UNPAIRING_REQUEST];
                [[self bleUnpairingCommand] doRequestBleConnectForUnpairing];
                break;
            default:
                // メイン画面に制御を戻す
                break;
        }
    }

    - (void)notifyCommandStartedWithCommandName:(NSString *)commandName {
        // コマンド開始メッセージを画面表示
        [self setCommandName:commandName];
        [self notifyCommandStarted:[self commandName]];
    }

#pragma mark - Call back from BLEPairingCommand

    - (void)doResponseBLEPairing:(bool)success message:(NSString *)message {
        // メイン画面に制御を戻す
        [self notifyCommandTerminated:[self commandName] message:message success:success fromWindow:[self parentWindow]];
    }

    - (void)notifyMessage:(NSString *)message {
        // メイン画面にテキストを表示
        [[self delegate] notifyMessageToMainUI:message];
    }

#pragma mark - Call back from BLEUnpairingCommand

    - (void)doResponseBleConnectForUnpairing:(bool)success message:(NSString *)message {
        // メイン画面に制御を戻す
        [self notifyCommandTerminated:[self commandName] message:message success:success fromWindow:[self parentWindow]];
    }

@end
