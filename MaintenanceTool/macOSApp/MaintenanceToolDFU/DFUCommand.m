//
//  DFUCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/20.
//
#import "AppCommonMessage.h"
#import "DFUCommand.h"
#import "DFUWindow.h"
#import "ToolBLEDFUCommand.h"
#import "ToolPopupWindow.h"
#import "USBDFUCommand.h"

@implementation DFUCommandParameter

@end

@interface DFUCommand () <USBDFUCommandDelegate, ToolBLEDFUCommandDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) DFUWindow                    *dfuWindow;
    // 下位クラスの参照を保持
    @property (nonatomic) USBDFUCommand                *usbDFUCommand;
    @property (nonatomic) ToolBLEDFUCommand            *toolBLEDFUCommand;
    // DFU処理のパラメーターを保持
    @property (nonatomic) DFUCommandParameter          *commandParameter;

@end

@implementation DFUCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // 画面のインスタンスを生成
            [self setDfuWindow:[[DFUWindow alloc] initWithWindowNibName:@"DFUWindow"]];
            // ヘルパークラスのインスタンスを生成
            [self setCommandParameter:[[DFUCommandParameter alloc] init]];
            [self setUsbDFUCommand:[[USBDFUCommand alloc] initWithDelegate:self]];
            [self setToolBLEDFUCommand:[[ToolBLEDFUCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (void)DFUWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self dfuWindow] setParentWindowRef:parentWindow withCommandRef:self withParameterRef:[self commandParameter]];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self dfuWindow] window];
        DFUCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf dfuWindowDidClose:self modalResponse:response];
        }];
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self usbDFUCommand] isUSBHIDConnected];
    }

#pragma mark - Perform functions

    - (void)dfuWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self dfuWindow] close];
        // 実行コマンドにより処理分岐
        switch ([[self commandParameter] transportType]) {
            case TRANSPORT_HID:
                // ファームウェア更新処理（USB）を実行
                [[self usbDFUCommand] usbDfuProcessWillStart:self parentWindow:[self parentWindow]];
                break;
            case TRANSPORT_BLE:
                // ファームウェア更新処理（BLE）を実行
                [[self toolBLEDFUCommand] bleDfuProcessWillStart:self parentWindow:[self parentWindow]];
                break;
            default:
                // メイン画面に制御を戻す
                break;
        }
    }

#pragma mark - Perform functions

    - (void)notifyCommandStartedWithCommandName:(NSString *)commandName {
        // コマンド開始メッセージを画面表示
        [self setCommandName:commandName];
        [super notifyCommandStarted:[self commandName]];
    }

#pragma mark - Call back from ToolBLEDFUCommand

    - (void)notifyCommandStarted:(Command)command {
        [self notifyCommandStartedWithCommandName:PROCESS_NAME_BLE_DFU];
    }

    - (void)notifyCommandTerminated:(Command)command success:(bool)success message:(NSString *)message {
        // メイン画面に制御を戻す
        NSString *commandName = (command == COMMAND_NONE) ? nil : [self commandName];
        [self notifyCommandTerminated:commandName message:message success:success fromWindow:[self parentWindow]];
    }

    - (void)notifyMessage:(NSString *)message {
        [[self delegate] notifyMessageToMainUI:message];
    }

#pragma mark - Call back from USBDFUCommand

    - (void)notifyCommandStartedWithCommand:(Command)command {
        [self notifyCommandStartedWithCommandName:PROCESS_NAME_USB_DFU];
    }

@end
