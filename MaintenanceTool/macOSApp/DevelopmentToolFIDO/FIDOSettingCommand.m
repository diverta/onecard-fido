//
//  FIDOSettingCommand.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/08.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "AppHIDCommand.h"
#import "FIDOAttestationWindow.h"
#import "FIDOSettingCommand.h"
#import "FIDOSettingWindow.h"
#import "ToolLogFile.h"

@interface FIDOSettingCommand () <AppHIDCommandDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) FIDOSettingWindow            *fidoSettingWindow;
    @property (nonatomic) FIDOAttestationWindow        *fidoAttestationWindow;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;

@end

@implementation FIDOSettingCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // 画面のインスタンスを生成
            [self setFidoSettingWindow:[[FIDOSettingWindow alloc] initWithWindowNibName:@"FIDOSettingWindow"]];
            [self setFidoAttestationWindow:[[FIDOAttestationWindow alloc] initWithWindowNibName:@"FIDOAttestationWindow"]];
            // このクラスの参照を引き渡し
            [[self fidoSettingWindow] setCommandRef:self];
            [[self fidoAttestationWindow] setCommandRef:self];
            // ヘルパークラスのインスタンスを生成
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (void)FIDOSettingWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self fidoSettingWindow] setParentWindowRef:parentWindow];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self fidoSettingWindow] window];
        FIDOSettingCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf fidoSettingWindowDidClose:self modalResponse:response];
        }];
    }

#pragma mark - Perform functions

    - (void)fidoSettingWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self fidoSettingWindow] close];
        // 実行コマンドにより処理分岐
        switch ([[self fidoSettingWindow] commandToPerform]) {
            case COMMAND_FIDO_ATTESTATION:
                // FIDO鍵・証明書インストール画面を表示
                [self fidoAttestationWindowWillOpen];
                break;
            case COMMAND_FIDO_ATTESTATION_RESET:
                // FIDO鍵・証明書を消去
                [self doFIDOAttestationReset];
                break;
            default:
                // メイン画面に制御を戻す
                break;
        }
    }

#pragma mark - For FIDO attestation window

    - (void)fidoAttestationWindowWillOpen {
        // 画面に親画面参照をセット
        NSWindow *parentWindow = [self parentWindow];
        [[self fidoAttestationWindow] setParentWindowRef:parentWindow];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self fidoAttestationWindow] window];
        FIDOSettingCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf fidoAttestationWindowDidClose:weakSelf modalResponse:response];
        }];
    }

    - (void)fidoAttestationWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self fidoAttestationWindow] close];
        // 実行コマンドにより処理分岐
        switch ([[self fidoAttestationWindow] commandToPerform]) {
            case COMMAND_FIDO_ATTESTATION_INSTALL:
                // FIDO鍵・証明書をインストール
                [self doFIDOAttestationInstall];
                break;
            default:
                // メイン画面に制御を戻す
                break;
        }
    }

#pragma mark - FIDO setting functions

    - (void)doFIDOAttestationInstall {
        // TODO: FIDO鍵・証明書をインストール
        [[self delegate] notifyMessageToMainUI:MSG_APP_FUNC_NOT_SUPPORTED];
    }

    - (void)doFIDOAttestationReset {
        // TODO: FIDO鍵・証明書を消去
        [[self delegate] notifyMessageToMainUI:MSG_APP_FUNC_NOT_SUPPORTED];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        return [[self appHIDCommand] checkUSBHIDConnection];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

@end
