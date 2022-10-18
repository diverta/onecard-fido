//
//  USBDFUCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/18.
//
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "DFUCommand.h"
#import "USBDFUCommand.h"

@interface USBDFUCommand () <AppHIDCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                      delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;
    // DFU処理のパラメーターを保持
    @property (nonatomic) DFUCommandParameter          *commandParameter;

@end

@implementation USBDFUCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 上位クラスの参照を保持
            [self setDelegate:delegate];
            // ヘルパークラスのインスタンスを生成
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
            // パラメーターを初期化
            [self setCommandParameter:[[DFUCommandParameter alloc] init]];
            [[self commandParameter] setDfuStatus:DFU_ST_NONE];
        }
        return self;
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self appHIDCommand] checkUSBHIDConnection];
    }

#pragma mark - Command/subcommand process

    - (void)usbDfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow {
        // TODO: 仮の実装です。
        [[self delegate] notifyCommandStarted:COMMAND_USB_DFU];
        [self usbDfuProcessDidCompleted:false message:MSG_CMDTST_MENU_NOT_SUPPORTED];
    }

    - (void)usbDfuProcessDidCompleted:(bool)result message:(NSString *)message {
        // 上位クラスに制御を戻す
        [[self delegate] notifyCommandTerminated:COMMAND_USB_DFU success:result message:message];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時で上位クラスに制御を戻す
        if (success == false) {
            [self usbDfuProcessDidCompleted:false message:errorMessage];
            return;
        }
        // 実行コマンドにより処理分岐
        switch (command) {
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self usbDfuProcessDidCompleted:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

@end
