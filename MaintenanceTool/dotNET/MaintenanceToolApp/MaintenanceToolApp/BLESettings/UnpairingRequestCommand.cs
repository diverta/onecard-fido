using ToolAppCommon;
using static MaintenanceToolApp.FIDODefine;

namespace MaintenanceToolApp.BLESettings
{
    internal class UnpairingRequestCommand
    {
        // 処理実行のためのプロパティー
        private BLESettingsParameter Parameter = null!;

        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyCommandTerminated(string commandTitle, string errorMessage, bool success);
        private event HandlerOnNotifyCommandTerminated NotifyCommandTerminated = null!;

        // トランスポートからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        public UnpairingRequestCommand(BLESettingsParameter parameter)
        {
            // パラメーターの参照を保持
            Parameter = parameter;

            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
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
            NotifyCommandTerminated(Parameter.CommandTitle, errorMessage, success);
        }

        private void DoRequestUnpairingCommand()
        {
            // コマンドバイトだけを送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestBleCommand(0x80 | FIDO_CMD_MSG, CommandDataForUnpairingRequest());
        }

        private void DoResponseUnpairingCommand(byte[] responseData)
        {
            // TODO: 仮の実装です。
            NotifyProcessTerminated(true, AppCommon.MSG_NONE);
        }

        private byte[] CommandDataForUnpairingRequest()
        {
            return new byte[] { MNT_COMMAND_UNPAIRING_REQUEST };
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
    }
}
