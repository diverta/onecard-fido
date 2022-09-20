namespace MaintenanceToolApp.BLESettings
{
    internal class PairingProcess
    {
        // 処理実行のためのプロパティー
        private readonly BLESettingsParameter Parameter;

        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyCommandTerminated(string commandTitle, string errorMessage, bool success);
        private event HandlerOnNotifyCommandTerminated NotifyCommandTerminated = null!;

        // HID／BLEからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        public PairingProcess(BLESettingsParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;

            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
        }

        //
        // 外部公開用
        //
        public void DoRequestPairing(HandlerOnNotifyCommandTerminated handler)
        {
            // 戻り先の関数を保持
            NotifyCommandTerminated = handler;

            // TODO: 仮の実装です。
            NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED, false);
        }

        //
        // HID／BLEからのレスポンス振分け処理
        //
        private void OnCommandResponse(byte CMD, byte[] responseData, bool success, string errorMessage)
        {
            // イベントを解除
            CommandProcess.UnregisterHandlerOnCommandResponse(OnCommandResponseRef);

            // 即時でアプリケーションに制御を戻す
            if (success == false) {
                NotifyCommandTerminated(Parameter.CommandTitle, errorMessage, success);
                return;
            }

            // 実行コマンドにより処理分岐
            switch (Parameter.Command) {
            default:
                // メイン画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
                break;
            }
        }
    }
}
