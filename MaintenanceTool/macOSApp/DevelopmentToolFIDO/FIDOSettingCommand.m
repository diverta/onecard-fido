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
#import "ToolCommonFunc.h"
#import "ToolLogFile.h"
#import "ToolPopupWindow.h"

@interface FIDOSettingCommand () <AppHIDCommandDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) FIDOSettingWindow            *fidoSettingWindow;
    @property (nonatomic) FIDOAttestationWindow        *fidoAttestationWindow;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;
    // 実行コマンドを保持
    @property (nonatomic) Command                       command;
    @property (nonatomic) NSString                     *commandName;

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
        // FIDO鍵・証明書をインストール
        [self setCommand:COMMAND_FIDO_ATTESTATION_INSTALL];
        [self setCommandName:PROCESS_NAME_INSTALL_SKEY_CERT];
        // コマンド開始メッセージを画面表示
        [self notifyCommandStarted:[self commandName]];
        // インストール処理を開始
        [[self appHIDCommand] doRequestCommand:[self command]];
    }

    - (void)doFIDOAttestationInstallRequest {
        // FIDO鍵・証明書インストール用リクエストデータを生成
        // 処理失敗時はその旨を画面に通知
        [self setCommand:COMMAND_FIDO_ATTESTATION_INSTALL_REQUEST];
        // インストールリクエストを実行
        [[self appHIDCommand] doRequestCommand:[self command]];
    }

    - (void)doFIDOAttestationReset {
        // TODO: FIDO鍵・証明書を消去
        [[self delegate] notifyMessageToMainUI:MSG_APP_FUNC_NOT_SUPPORTED];
    }

    - (bool)checkUSBHIDConnectionOnWindow:(NSWindow *)window {
        // USBポートに接続されていない場合はfalse
        bool connected = [[self appHIDCommand] checkUSBHIDConnection];
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:window connected:connected];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時でアプリケーションに制御を戻す
        if (success == false) {
            [self notifyCommandTerminated:[self commandName] message:errorMessage success:success fromWindow:[self parentWindow]];
            return;
        }
        // 実行コマンドにより処理分岐
        switch (command) {
            case COMMAND_FIDO_ATTESTATION_INSTALL:
                // FIDO鍵・証明書インストール用リクエストデータを生成
                [self doFIDOAttestationInstallRequest];
                break;
            default:
                // メイン画面に制御を戻す
                [self notifyCommandTerminated:[self commandName] message:nil success:success fromWindow:[self parentWindow]];
                break;
        }
    }

@end
