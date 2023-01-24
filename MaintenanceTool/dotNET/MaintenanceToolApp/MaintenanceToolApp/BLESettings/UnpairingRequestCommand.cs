using System;
using System.Threading;
using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.FIDODefine;

namespace MaintenanceToolApp.BLESettings
{
    internal class UnpairingRequestCommand
    {
        // Bluetooth環境設定からデバイスが削除されるのを待機する時間（秒）
        private const int UNPAIRING_REQUEST_WAITING_SEC = 30;

        // 処理実行のためのプロパティー
        private BLESettingsParameter Parameter = null!;

        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyCommandTerminated(string commandTitle, string errorMessage, bool success);
        private event HandlerOnNotifyCommandTerminated NotifyCommandTerminated = null!;

        // トランスポートからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        // BLE接続／切断検知時のコールバック参照
        private readonly CommandProcess.HandlerNotifyBLEConnectionStatus NotifyBLEConnectionStatusRef;

        // タイムアウト監視フラグ
        private bool WaitingForUnpairTimeout { get; set; }

        public UnpairingRequestCommand(BLESettingsParameter parameter)
        {
            // パラメーターの参照を保持
            Parameter = parameter;

            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
            NotifyBLEConnectionStatusRef = new CommandProcess.HandlerNotifyBLEConnectionStatus(NotifyBLEConnectionStatus);

            // フラグの初期化
            WaitingForUnpairTimeout = false;
        }

        public void DoUnpairingRequestProcess(HandlerOnNotifyCommandTerminated handlerRef)
        {
            // コールバックを保持
            NotifyCommandTerminated = handlerRef;

            // ペアリング解除要求コマンドを実行
            DoRequestUnpairingCommand();
        }

        // 
        // 共通処理
        //
        private void NotifyProcessTerminated(bool success, string errorMessage)
        {
            // BLE接続を破棄
            BLEProcess.DisconnctBLE();

            // 画面に制御を戻す
            Application.Current.Dispatcher.Invoke(new Action(() => {
                NotifyCommandTerminated(Parameter.CommandTitle, errorMessage, success);
            }));
        }

        private void DoRequestUnpairingCommand()
        {
            // コマンドバイトだけを送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestBleCommand(0x80 | FIDO_CMD_MSG, CommandDataForUnpairingRequest(null!));
        }

        private void DoResponseUnpairingCommand(byte[] responseData)
        {
            if (responseData.Length == 3) {
                // レスポンスにpeer_idが設定されている場合は次のコマンドを実行
                DoRequestUnpairingCommandWithPeerId(responseData);

            } else {
                // レスポンスがブランクの場合は、ペアリング解除による切断 or タイムアウト／キャンセル応答まで待機
                StartWaitingForUnpair();

                // タイムアウト監視に移行
                WaitingForUnpairTimeout = true;
                StartWaitingForUnpairTimeoutMonitor();
            }
        }

        private void DoRequestUnpairingCommandWithPeerId(byte[] responseData)
        {
            // コマンドバイトにpeer_idを付加して送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestBleCommand(0x80 | FIDO_CMD_MSG, CommandDataForUnpairingRequest(responseData));
        }

        private static byte[] CommandDataForUnpairingRequest(byte[] data)
        {
            if (data == null) {
                return new byte[] { MNT_COMMAND_UNPAIRING_REQUEST };

            } else {
                // ペアリング解除要求コマンド用のデータを生成（レスポンスの２・３バイト目＝peer_idを設定）
                return new byte[] { MNT_COMMAND_UNPAIRING_REQUEST, data[1], data[2] };
            }
        }

        private void StartWaitingForUnpairTimeoutMonitor()
        {
            // タイムアウト監視（最大30秒）
            for (int i = 0; i < UNPAIRING_REQUEST_WAITING_SEC; i++) {
                // 残り秒数をペアリング解除要求画面に通知
                int sec = UNPAIRING_REQUEST_WAITING_SEC - i;
                for (int j = 0; j < 5; j++) {
                    if (WaitingForUnpairTimeout == false) {
                        return;
                    }
                    Thread.Sleep(200);
                }
            }

            // タイムアウトと判定
            // TODO: 仮の実装です。
            NotifyProcessTerminated(false, AppCommon.MSG_BLE_UNPAIRING_WAIT_CANCELED);
        }

        private void CancelWaitingForUnpairTimeoutMonitor()
        {
            // タイムアウト監視を停止
            WaitingForUnpairTimeout = false;
        }

        //
        // トランスポートからのレスポンス振分け処理
        //
        private void OnCommandResponse(byte CMD, byte[] responseData, bool success, string errorMessage)
        {
            // イベントを解除
            CommandProcess.UnregisterHandlerOnCommandResponse(OnCommandResponseRef);

            // 即時でアプリケーションに制御を戻す
            if (success == false) {
                NotifyProcessTerminated(false, errorMessage);
                return;
            }

            // レスポンスメッセージの１バイト目（ステータスコード）を確認
            if (responseData[0] != 0x00) {
                // エラーの場合は処理異常終了
                NotifyProcessTerminated(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                return;
            }

            // 処理正常終了
            DoResponseUnpairingCommand(responseData);
        }

        //
        // BLE接続／切断検知時の処理
        //
        private void StartWaitingForUnpair()
        {
            // BLE接続／切断検知時のコールバックを設定
            CommandProcess.RegisterHandlerNotifyBLEConnectionStatus(NotifyBLEConnectionStatusRef);
        }

        private void NotifyBLEConnectionStatus(bool connected)
        {
            // イベントを解除
            CommandProcess.UnregisterHandlerNotifyBLEConnectionStatus(NotifyBLEConnectionStatusRef);

            if (connected == false) {
                // ペアリング解除による切断検知時
                // タイムアウト監視を停止
                CancelWaitingForUnpairTimeoutMonitor();

                // TODO: 仮の実装です。
                NotifyProcessTerminated(true, AppCommon.MSG_NONE);
            }
        }
    }
}
