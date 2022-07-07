//
//  AppHIDCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/07.
//
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "FIDODefines.h"
#import "ToolCommonMessage.h"
#import "ToolHIDHelper.h"

@interface AppHIDCommand () <ToolHIDHelperDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id          delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) ToolHIDHelper    *toolHIDHelper;
    // 実行コマンドを保持
    @property (nonatomic) Command           command;

@end

@implementation AppHIDCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
            [self setToolHIDHelper:[[ToolHIDHelper alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        return [[self toolHIDHelper] isDeviceConnected];
    }

#pragma mark - HID channel functions

    static char cidBytes[] = {0xff, 0xff, 0xff, 0xff};

    - (void)doRequestCommand:(Command)command withCMD:(uint8_t)cmd withData:(NSData *)data {
        // 実行コマンドを保持
        [self setCommand:command];
        // HIDコマンド／データを送信（CIDはダミーを使用する）
        NSData *cid = [[NSData alloc] initWithBytes:cidBytes length:sizeof(cidBytes)];
        [[self toolHIDHelper] hidHelperWillSend:data CID:cid CMD:cmd];
    }

#pragma mark - Call back from ToolHIDHelper

    - (void)hidHelperDidDetectConnect {
        [[self delegate] didDetectConnect];
    }

    - (void)hidHelperDidDetectRemoval {
        [[self delegate] didDetectRemoval];
    }

    - (void)hidHelperDidReceive:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *requestBytes = (uint8_t *)[message bytes];
        if (requestBytes[0] != CTAP1_ERR_SUCCESS) {
            // エラーの場合は画面に制御を戻す
            [[self delegate] didResponseCommand:[self command] response:message success:false errorMessage:MSG_OCCUR_UNKNOWN_ERROR];
            return;
        }
        // 正常終了扱い
        [[self delegate] didResponseCommand:[self command] response:message success:true errorMessage:nil];
    }

    - (void)hidHelperDidResponseTimeout {
        // タイムアウト時はエラーメッセージを表示
        [[self delegate] didResponseCommand:[self command] response:nil success:false errorMessage:MSG_HID_CMD_RESPONSE_TIMEOUT];
    }

@end
