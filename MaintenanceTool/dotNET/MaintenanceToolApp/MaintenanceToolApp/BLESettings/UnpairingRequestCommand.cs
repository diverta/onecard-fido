namespace MaintenanceToolApp.BLESettings
{
    internal class UnpairingRequestCommand
    {
        // 処理実行のためのプロパティー
        private BLESettingsParameter Parameter = null!;

        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyCommandTerminated(string commandTitle, string errorMessage, bool success);
        private event HandlerOnNotifyCommandTerminated NotifyCommandTerminated = null!;

        public UnpairingRequestCommand(BLESettingsParameter parameter)
        {
            // パラメーターの参照を保持
            Parameter = parameter;
        }

        public void DoUnpairingRequestProcess(HandlerOnNotifyCommandTerminated handlerRef)
        {
            // コールバックを保持
            NotifyCommandTerminated = handlerRef;

            // TODO: 仮の実装です。
            // 画面に制御を戻す
            NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_NONE, true);
        }
    }
}
