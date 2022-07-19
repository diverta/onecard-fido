//
//  FIDOSettingCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/18.
//
#import "AppCommonMessage.h"
#import "CTAP2ManageCommand.h"
#import "FIDOSettingCommand.h"
#import "FIDOSettingWindow.h"

@implementation FIDOSettingCommandParameter

@end

@interface FIDOSettingCommand () <CTAP2ManageCommandDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) FIDOSettingWindow            *fidoSettingWindow;
    // 下位クラスの参照を保持
    @property (nonatomic) CTAP2ManageCommand           *ctap2ManageCommand;
    // PIN番号管理処理のパラメーターを保持
    @property (nonatomic) FIDOSettingCommandParameter  *commandParameter;

@end

@implementation FIDOSettingCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // 画面のインスタンスを生成
            [self setFidoSettingWindow:[[FIDOSettingWindow alloc] initWithWindowNibName:@"FIDOSettingWindow"]];
            // ヘルパークラスのインスタンスを生成
            [self setCommandParameter:[[FIDOSettingCommandParameter alloc] init]];
            [self setCtap2ManageCommand:[[CTAP2ManageCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (void)fidoSettingWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self fidoSettingWindow] setParentWindowRef:parentWindow withCommandRef:self withParameterRef:[self commandParameter]];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self fidoSettingWindow] window];
        FIDOSettingCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf fidoSettingWindowDidClose:self modalResponse:response];
        }];
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self ctap2ManageCommand] isUSBHIDConnected];
    }

#pragma mark - Perform functions

    - (void)fidoSettingWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self fidoSettingWindow] close];
        // 実行コマンドにより処理分岐
        switch ([[self commandParameter] command]) {
            case COMMAND_CLIENT_PIN_SET:
                [self notifyCommandStartedWithCommandName:PROCESS_NAME_CLIENT_PIN_SET];
                [[self ctap2ManageCommand] doRequestHidCtap2Management:[self commandParameter]];
                break;
            case COMMAND_CLIENT_PIN_CHANGE:
                [self notifyCommandStartedWithCommandName:PROCESS_NAME_CLIENT_PIN_CHANGE];
                [[self ctap2ManageCommand] doRequestHidCtap2Management:[self commandParameter]];
                break;
            case COMMAND_AUTH_RESET:
                [self notifyCommandStartedWithCommandName:PROCESS_NAME_AUTH_RESET];
                [[self ctap2ManageCommand] doRequestHidCtap2Management:[self commandParameter]];
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

#pragma mark - Call back from U2FHcheckCommand

    - (void)doResponseCtap2Management:(bool)success message:(NSString *)message {
        // メイン画面に制御を戻す
        [self notifyCommandTerminated:[self commandName] message:message success:success fromWindow:[self parentWindow]];
    }

    - (void)notifyMessage:(NSString *)message {
        // メイン画面にテキストを表示
        [[self delegate] notifyMessageToMainUI:message];
    }

@end
