//
//  BLEUnpairingCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/12/09.
//
#import "AppBLECommand.h"
#import "AppCommonMessage.h"
#import "BLEUnpairingCommand.h"
#import "BLEUnpairingDefine.h"
#import "FIDODefines.h"
#import "UnpairingRequestWindow.h"

@interface BLEUnpairingCommand () <AppBLECommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                  delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppBLECommand            *appBLECommand;
    // 画面の参照を保持
    @property (nonatomic) UnpairingRequestWindow   *unpairingRequestWindow;
    // 非同期処理用のキュー（画面用／待機処理用）
    @property (nonatomic) dispatch_queue_t          mainQueue;
    @property (nonatomic) dispatch_queue_t          subQueue;
    // 切断待機フラグ
    @property (nonatomic) bool                      waitingDisconnect;
    // タイムアウト監視フラグ
    @property (nonatomic) bool                      WaitingForUnpairTimeout;
    // メイン画面に戻すエラーメッセージを保持
    @property (nonatomic) NSString                 *commandErrorMessage;

@end

@implementation BLEUnpairingCommand

    - (id)init {
        return [self initWithDelegate:nil withUnpairingRequestWindowRef:nil];
    }

    - (id)initWithDelegate:(id)delegate withUnpairingRequestWindowRef:(id)windowRef {
        self = [super init];
        if (self) {
            // 上位クラスの参照を保持
            [self setDelegate:delegate];
            // ヘルパークラスのインスタンスを生成
            [self setAppBLECommand:[[AppBLECommand alloc] initWithDelegate:self]];
            // 画面の参照を保持
            [self setUnpairingRequestWindow:(UnpairingRequestWindow *)windowRef];
            // メインスレッド／サブスレッドにバインドされるデフォルトキューを取得
            [self setMainQueue:dispatch_get_main_queue()];
            [self setSubQueue:dispatch_queue_create("jp.co.diverta.fido.maintenancetool.ble.unpair", DISPATCH_QUEUE_SERIAL)];
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
            // エラーの場合はヘルパークラスに制御を戻す-->BLE切断後、didCompleteCommand-->terminateUnpairingCommand が呼び出される
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
        // 切断待機フラグを設定
        [self setWaitingDisconnect:true];
        dispatch_async([self mainQueue], ^{
            // ペアリング解除要求画面にデバイス名を通知
            [[self unpairingRequestWindow] commandDidStartWaitingForUnpairWithDeviceName:[[self appBLECommand] nameOfScannedPeripheral]];
        });
        // タイムアウト監視に移行
        dispatch_async([self subQueue], ^{
            [self setWaitingForUnpairTimeout:true];
            [self startWaitingForUnpairTimeoutMonitor];
        });
    }

    - (void)doRequestUnpairingCancelCommand {
        // ペアリング解除要求キャンセルコマンド用のデータを生成
        unsigned char arr[] = {MNT_COMMAND_UNPAIRING_CANCEL};
        NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
        // ペアリング解除要求キャンセルコマンドを実行
        [[self appBLECommand] doRequestCommand:COMMAND_UNPAIRING_CANCEL_REQUEST withCMD:BLE_CMD_MSG withData:commandData];
    }

    - (void)doResponseUnpairingCancelCommand {
        // 切断待機フラグをクリア
        [self setWaitingDisconnect:false];
        // 一旦ヘルパークラスに制御を戻す-->BLE切断後、didCompleteCommand-->terminateUnpairingCommand が呼び出される
        [[self appBLECommand] commandDidProcess:false message:MSG_BLE_UNPAIRING_WAIT_CANCELED];
    }

    - (void)terminateUnpairingCommand:(bool)success message:(NSString *)message {
        // メッセージを退避
        [self setCommandErrorMessage:message];
        dispatch_async([self mainQueue], ^{
            // ペアリング解除要求画面を閉じる-->unpairingRequestWindowDidClose が呼び出される
            if ([message isEqualToString:MSG_BLE_UNPAIRING_WAIT_CANCELED]) {
                [[self unpairingRequestWindow] commandDidCancelUnpairingRequestProcess];
            } else {
                [[self unpairingRequestWindow] commandDidTerminateUnpairingRequestProcess:success];
            }
        });
    }

#pragma mark - Waiting for unpair Timeout Monitor

    - (void)startWaitingForUnpairTimeoutMonitor {
        // タイムアウト監視（最大30秒）
        for (int i = 0; i < UNPAIRING_REQUEST_WAITING_SEC; i++) {
            // 残り秒数をペアリング解除要求画面に通知
            int sec = UNPAIRING_REQUEST_WAITING_SEC - i;
            [self notifyProgressValue:sec];
            for (int j = 0; j < 5; j++) {
                if ([self WaitingForUnpairTimeout] == false) {
                    return;
                }
                [NSThread sleepForTimeInterval:0.2];
            }
        }
        // 残り秒数をペアリング解除要求画面に通知
        [self notifyProgressValue:0];
        // タイムアウトと判定
        [self waitingForUnpairTimeoutMonitorDidTimeout];
    }

    - (void)notifyProgressValue:(int)remaining {
        dispatch_async([self mainQueue], ^{
            // 残り秒数をペアリング解除要求画面に通知
            NSString *message = [NSString stringWithFormat:MSG_BLE_UNPAIRING_WAIT_SEC_FORMAT, remaining];
            [[self unpairingRequestWindow] commandDidNotifyProcessWithMessage:message withProgress:remaining];
        });
    }

    - (void)cancelWaitingForUnpairTimeoutMonitor {
        // タイムアウト監視を停止
        [self setWaitingForUnpairTimeout:false];
    }

    - (void)waitingForUnpairTimeoutMonitorDidTimeout {
        // ペアリング解除要求キャンセルコマンドを実行
        [self doRequestUnpairingCancelCommand];
    }

#pragma mark - Call back from AppBLECommand

    - (void)didResponseCommand:(Command)command response:(NSData *)response {
        if (command == COMMAND_UNPAIRING_REQUEST) {
            [self doResponseUnpairingCommand:response];
        }
        if (command == COMMAND_UNPAIRING_CANCEL_REQUEST) {
            [self doResponseUnpairingCancelCommand];
        }
    }

    - (void)didCompleteCommand:(Command)command success:(bool)success errorMessage:(NSString *)errorMessage {
        if ([self waitingDisconnect]) {
            // 切断待機フラグをクリア
            [self setWaitingDisconnect:false];
            // タイムアウト監視を停止
            [self cancelWaitingForUnpairTimeoutMonitor];
            // 一旦ヘルパークラスに制御を戻す-->BLE切断後、didCompleteCommand が呼び出される
            [[self appBLECommand] commandDidProcess:true message:nil];
            return;
        }
        // ペアリング解除要求画面を閉じ、上位クラスに制御を戻す
        [self terminateUnpairingCommand:success message:errorMessage];
    }

#pragma mark - Interface for UnpairingRequestWindow

    - (void)invokeUnpairingRequestProcess {
        // ペアリング解除要求画面（ダイアログ）をモーダルで表示
        [self unpairingRequestWindowWillOpen];
        // キャンセルボタンがクリックされた時に実行されるコールバック、待機秒数を設定
        [[self unpairingRequestWindow] commandDidStartUnpairingRequestProcessForTarget:self
            forSelector:@selector(unpairingRequestWindowNotifyCancel) withProgressMax:UNPAIRING_REQUEST_WAITING_SEC];
        // ペアリング解除要求コマンドを実行
        [self doRequestUnpairingCommand];
    }

    - (void)unpairingRequestWindowWillOpen {
        NSWindow *dialog = [[self unpairingRequestWindow] window];
        BLEUnpairingCommand * __weak weakSelf = self;
        [[[self unpairingRequestWindow] parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf unpairingRequestWindowDidClose:self modalResponse:response];
        }];
    }

    - (void)unpairingRequestWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // ペアリング解除要求画面を閉じる
        [[self unpairingRequestWindow] close];
        // 上位クラスに制御を戻す
        switch (modalResponse) {
            case NSModalResponseOK:
                [[self delegate] doResponseBLESettingCommand:true message:nil];
                break;
            default:
                [[self delegate] doResponseBLESettingCommand:false message:[self commandErrorMessage]];
                break;
        }
        
    }

    - (void)unpairingRequestWindowNotifyCancel {
        // タイムアウト監視を停止
        [self cancelWaitingForUnpairTimeoutMonitor];
        // ペアリング解除要求画面のCancelボタンがクリックされた場合、ペアリング解除要求キャンセルコマンドを実行
        [self doRequestUnpairingCancelCommand];
    }

@end
