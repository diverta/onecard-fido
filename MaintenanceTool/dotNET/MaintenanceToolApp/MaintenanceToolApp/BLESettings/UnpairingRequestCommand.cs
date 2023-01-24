using System;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.FIDODefine;

namespace MaintenanceToolApp.BLESettings
{
    internal class UnpairingRequestParameter
    {
        // リクエストデータ
        public byte[] CommandData { get; set; }

        // タイムアウト監視フラグ
        public bool WaitingForUnpairTimeout { get; set; }

        // 実行する管理コマンドバイトを保持
        public byte MaintenanceCMD { get; set; }

        public UnpairingRequestParameter()
        {
            CommandData = Array.Empty<byte>();
            WaitingForUnpairTimeout = false;
            MaintenanceCMD = 0x00;
        }
    }

    internal class UnpairingRequestCommand
    {
        // ペアリング解除要求画面の親ウィンドウの参照を保持
        private readonly Window ParentWindow = App.Current.MainWindow;

        // ペアリング解除要求画面の参照を保持
        private UnpairingRequestWindow UnpairingRequestWindowRef = null!;

        // Bluetooth環境設定からデバイスが削除されるのを待機する時間（秒）
        public const int UNPAIRING_REQUEST_WAITING_SEC = 30;

        // 処理実行のためのプロパティー
        private BLESettingsParameter Parameter = null!;

        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyCommandTerminated(string commandTitle, string errorMessage, bool success);
        private event HandlerOnNotifyCommandTerminated NotifyCommandTerminated = null!;

        // トランスポートからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        // BLE接続／切断検知時のコールバック参照
        private readonly CommandProcess.HandlerNotifyBLEConnectionStatus NotifyBLEConnectionStatusRef;

        // リクエストパラメーターを保持
        private UnpairingRequestParameter RequestParameter { get; set; }

        public UnpairingRequestCommand(BLESettingsParameter parameter)
        {
            // パラメーターの参照を保持
            Parameter = parameter;

            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
            NotifyBLEConnectionStatusRef = new CommandProcess.HandlerNotifyBLEConnectionStatus(NotifyBLEConnectionStatus);

            // リクエストパラメーターの初期化
            RequestParameter = new UnpairingRequestParameter();
        }

        public void DoUnpairingRequestProcess(HandlerOnNotifyCommandTerminated handlerRef)
        {
            // コールバックを保持
            NotifyCommandTerminated = handlerRef;

            // ペアリング解除要求コマンドを実行
            DoRequestUnpairingCommand();

            // ペアリング解除要求画面を表示
            UnpairingRequestWindowRef = new UnpairingRequestWindow(Parameter);
            bool success = UnpairingRequestWindowRef.ShowDialogWithOwner(ParentWindow);

            // 親画面に制御を戻す
            Application.Current.Dispatcher.Invoke(new Action(() => {
                NotifyCommandTerminated(Parameter.CommandTitle, Parameter.ErrorMessage, success);
            }));
        }

        // 
        // 共通処理
        //
        private void NotifyProcessTerminated(bool success, string errorMessage)
        {
            // BLE接続を破棄
            BLEProcess.DisconnctBLE();

            // エラーメッセージを設定
            Parameter.ErrorMessage = errorMessage;

            // ペアリング解除要求画面を閉じる
            Application.Current.Dispatcher.Invoke(new Action(() => {
                UnpairingRequestWindowRef.CommandDidCancelUnpairingRequestProcess(success);
            }));
        }

        private void DoRequestUnpairingCommand()
        {
            // ペアリング解除要求コマンド用のデータを生成
            CommandDataForUnpairingRequest(null!);

            // ペアリング解除要求コマンドを実行
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestBleCommand(0x80 | FIDO_CMD_MSG, RequestParameter.CommandData);
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
                Task task = Task.Run(() => {
                    StartWaitingForUnpairTimeoutMonitor();
                });
            }
        }

        private void DoRequestUnpairingCommandWithPeerId(byte[] responseData)
        {
            // ペアリング解除要求コマンド用のデータを生成
            CommandDataForUnpairingRequest(responseData);

            // ペアリング解除要求コマンドを実行
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestBleCommand(0x80 | FIDO_CMD_MSG, RequestParameter.CommandData);
        }

        private void CommandDataForUnpairingRequest(byte[] data)
        {
            // 実行コマンドバイトを保持
            RequestParameter.MaintenanceCMD = MNT_COMMAND_UNPAIRING_REQUEST;

            if (data == null) {
                // ペアリング解除要求コマンド用のデータを生成
                RequestParameter.CommandData = new byte[] { RequestParameter.MaintenanceCMD };

            } else {
                // ペアリング解除要求コマンド用のデータを生成（レスポンスの２・３バイト目＝peer_idを設定）
                RequestParameter.CommandData = new byte[] { RequestParameter.MaintenanceCMD, data[1], data[2] };
            }
        }

        private void DoRequestUnpairingCancelCommand()
        {
            // ペアリング解除要求キャンセルコマンド用のデータを生成
            CommandDataForUnpairingCancel();

            // ペアリング解除要求キャンセルコマンドを実行
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestBleCommand(0x80 | FIDO_CMD_MSG, RequestParameter.CommandData);
        }

        private void DoResponseUnpairingCancelCommand()
        {
            // タイムアウト監視を停止
            CancelWaitingForUnpairTimeoutMonitor();

            // コマンドメッセージを設定
            NotifyProcessTerminated(false, AppCommon.MSG_BLE_UNPAIRING_WAIT_CANCELED);
        }

        private void CommandDataForUnpairingCancel()
        {
            // 実行コマンドバイトを保持
            RequestParameter.MaintenanceCMD = MNT_COMMAND_UNPAIRING_CANCEL;

            // ペアリング解除要求キャンセルコマンド用のデータを生成
            RequestParameter.CommandData = new byte[] { RequestParameter.MaintenanceCMD };
        }

        //
        // ペアリング解除要求からペアリング解除による切断検知までの
        // タイムアウト監視
        //
        private void StartWaitingForUnpairTimeoutMonitor()
        {
            // タイムアウト監視を開始
            RequestParameter.WaitingForUnpairTimeout = true;

            // タイムアウト監視（最大30秒）
            for (int i = 0; i < UNPAIRING_REQUEST_WAITING_SEC; i++) {
                // 残り秒数をペアリング解除要求画面に通知
                int sec = UNPAIRING_REQUEST_WAITING_SEC - i;
                for (int j = 0; j < 5; j++) {
                    if (RequestParameter.WaitingForUnpairTimeout == false) {
                        return;
                    }
                    Thread.Sleep(200);
                }
            }

            // タイムアウトと判定
            WaitingForUnpairTimeoutMonitorDidTimeout();
        }

        private void CancelWaitingForUnpairTimeoutMonitor()
        {
            // タイムアウト監視を停止
            RequestParameter.WaitingForUnpairTimeout = false;
        }

        private void WaitingForUnpairTimeoutMonitorDidTimeout()
        {
            // ペアリング解除要求キャンセルコマンドを実行
            DoRequestUnpairingCancelCommand();
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
                string msg = string.Format(AppCommon.MSG_OCCUR_UNKNOWN_ERROR_ST, responseData[0]);
                NotifyProcessTerminated(false, msg);
                return;
            }

            // 処理正常終了
            if (RequestParameter.MaintenanceCMD == MNT_COMMAND_UNPAIRING_REQUEST) {
                DoResponseUnpairingCommand(responseData);

            } else if (RequestParameter.MaintenanceCMD == MNT_COMMAND_UNPAIRING_CANCEL) {
                DoResponseUnpairingCancelCommand();
            }
        }

        //
        // BLE接続／切断検知時の処理
        //
        private void StartWaitingForUnpair()
        {
            // デバイス名を取得
            // TODO: 仮の実装です。
            AppLogUtil.OutputLogDebug(BLEProcess.ConnectedDeviceName());

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

                // ペアリング解除要求画面を閉じ、上位クラスに制御を戻す
                NotifyProcessTerminated(true, AppCommon.MSG_NONE);
            }
        }
    }
}
