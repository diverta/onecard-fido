namespace MaintenanceToolApp
{
    internal class UtilityProcess
    {
        // このクラスのインスタンス
        private static readonly UtilityProcess Instance = new UtilityProcess();

        // 処理実行のためのプロパティー
        private string CommandTitle = string.Empty;

        // HID接続完了時のイベント
        public delegate void HandlerOnNotifyMessageText(string messageText);
        public event HandlerOnNotifyMessageText OnNotifyMessageText = null!;

        //
        // 外部公開用
        //
        public static void RegisterHandlerOnNotifyMessageText(HandlerOnNotifyMessageText handler)
        {
            Instance.OnNotifyMessageText += handler;
        }

        public static void SetCommandTitle(string commandTitle)
        {
            Instance.CommandTitle = commandTitle;
        }

        public static void DoProcess()
        {
            Instance.DoUtilityProcess();
        }

        //
        // 内部処理
        //
        public void DoUtilityProcess()
        {
            switch (CommandTitle) {
            default:
                break;
            }

            // 処理完了を通知
            OnNotifyMessageText("これは仮の実装です。");
        }
    }
}
