//
//  BLEUnpairingCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/12/09.
//
#import "AppBLECommand.h"
#import "AppCommonMessage.h"
#import "BLEUnpairingCommand.h"
#import "FIDODefines.h"
#import "ToolLogFile.h"

@interface BLEUnpairingCommand () <AppBLECommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                  delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppBLECommand            *appBLECommand;
    // 切断待機フラグ
    @property (nonatomic) bool                      waitingDisconnect;

@end

@implementation BLEUnpairingCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 上位クラスの参照を保持
            [self setDelegate:delegate];
            // ヘルパークラスのインスタンスを生成
            [self setAppBLECommand:[[AppBLECommand alloc] initWithDelegate:self]];
        }
        return self;
    }

#pragma mark - BLE Command/subcommand process

    - (void)doRequestUnpairingCommand {
        // 切断待機フラグをクリア
        [self setWaitingDisconnect:false];
        // ペアリング解除要求コマンド用のデータを生成
        uint8_t arr[] = {MNT_COMMAND_UNPAIRING_REQUEST};
        NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
        // ペアリング解除要求コマンドを実行
        [[self appBLECommand] doRequestCommand:COMMAND_UNPAIRING_REQUEST withCMD:BLE_CMD_MSG withData:commandData];
    }

    - (void)doRequestUnpairingCommandWithPeerId:(NSData *)response {
        // ペアリング解除要求コマンド用のデータを生成（レスポンスの２・３バイト目＝peer_idを設定）
        uint8_t *responseBytes = (uint8_t *)[response bytes];
        uint8_t arr[] = {MNT_COMMAND_UNPAIRING_REQUEST, responseBytes[1], responseBytes[2]};
        NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
        // ペアリング解除要求コマンドを実行
        [[self appBLECommand] doRequestCommand:COMMAND_UNPAIRING_REQUEST withCMD:BLE_CMD_MSG withData:commandData];
    }

    - (void)doResponseUnpairingCommand:(NSData *)response {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *responseBytes = (uint8_t *)[response bytes];
        if (responseBytes[0] != CTAP1_ERR_SUCCESS) {
            // エラーの場合はヘルパークラスに制御を戻す
            [[self appBLECommand] commandDidProcess:false message:MSG_OCCUR_UNKNOWN_ERROR];
            return;
        }
        if ([response length] == 3) {
            // レスポンスにpeer_idが設定されている場合は次のコマンドを実行
            [self doRequestUnpairingCommandWithPeerId:response];
        } else {
            // レスポンスがブランクの場合は、ペアリング解除による切断 or タイムアウト／キャンセル応答まで待機
            [self startWaitingForUnpair];
        }
    }

    - (void)startWaitingForUnpair {
        // メイン画面にメッセージを表示
        NSString *message = [NSString stringWithFormat:MSG_BLE_UNPARING_WAIT_DISCONNECT, [[self appBLECommand] nameOfScannedPeripheral]];
        [[self delegate] notifyCommandMessageToMainUI:message];
        [[ToolLogFile defaultLogger] info:message];
        // タイムアウト監視を開始
        [self startWaitingForUnpairTimeoutMonitor];
        // 切断待機フラグを設定
        [self setWaitingDisconnect:true];
    }

    - (void)terminateUnpairingCommand:(bool)success message:(NSString *)message {
        // タイムアウト監視を終了
        [self cancelWaitingForUnpairTimeoutMonitor];
        // 上位クラスに制御を戻す
        [[self delegate] doResponseBLESettingCommand:success message:message];
    }

#pragma mark - Waiting for unpair Timeout Monitor

    - (void)startWaitingForUnpairTimeoutMonitor {
        // タイムアウト監視を開始（30秒後にタイムアウト）
        [self performSelector:@selector(waitingForUnpairTimeoutMonitorDidTimeout) withObject:nil afterDelay:30.0];
    }

    - (void)cancelWaitingForUnpairTimeoutMonitor {
        // タイムアウト監視を停止
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(waitingForUnpairTimeoutMonitorDidTimeout) object:nil];
    }

    - (void)waitingForUnpairTimeoutMonitorDidTimeout {
        // 切断待機フラグをクリア
        [self setWaitingDisconnect:false];
        // 一旦ヘルパークラスに制御を戻す-->BLE切断後、didCompleteCommand が呼び出される
        [[self appBLECommand] commandDidProcess:false message:MSG_BLE_UNPARING_WAIT_DISC_TIMEOUT];
    }

#pragma mark - Call back from AppBLECommand

    - (void)didResponseCommand:(Command)command response:(NSData *)response {
        if (command == COMMAND_UNPAIRING_REQUEST) {
            [self doResponseUnpairingCommand:response];
        }
    }

    - (void)didCompleteCommand:(Command)command success:(bool)success errorMessage:(NSString *)errorMessage {
        if ([self waitingDisconnect]) {
            // 切断待機フラグをクリア
            [self setWaitingDisconnect:false];
            // 一旦ヘルパークラスに制御を戻す-->BLE切断後、didCompleteCommand が呼び出される
            [[self appBLECommand] commandDidProcess:true message:nil];
            return;
        }
        // 上位クラスに制御を戻す
        [self terminateUnpairingCommand:success message:errorMessage];
    }

@end
