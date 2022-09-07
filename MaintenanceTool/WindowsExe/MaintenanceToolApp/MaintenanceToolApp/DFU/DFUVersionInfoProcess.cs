using ToolAppCommon;

namespace MaintenanceToolApp.DFU
{
    internal class DFUVersionInfoProcess
    {
        // このクラスのインスタンス
        private static readonly DFUVersionInfoProcess Instance = new DFUVersionInfoProcess();

        private DFUVersionInfoProcess()
        {
            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
        }

        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyCommandTerminated(bool success, string errorMessage);
        private event HandlerOnNotifyCommandTerminated NotifyCommandTerminated = null!;

        // BLEからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        //
        // 外部公開用
        //
        public static void DoRequestVersionInfo(HandlerOnNotifyCommandTerminated handler)
        {
            // 戻り先の関数を保持
            Instance.NotifyCommandTerminated = handler;

            // バージョン照会コマンドを実行
            Instance.DoRequestBLEGetVersionInfo();
        }

        //
        // 内部処理
        //
        private void DoRequestBLEGetVersionInfo()
        {
            // コマンドバイトだけを送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestBleCommand(HIDProcessConst.HID_CMD_GET_VERSION_INFO, new byte[1]);
        }

        //
        // BLEからのレスポンス振分け処理
        //
        private void OnCommandResponse(byte CMD, byte[] responseData, bool success, string errorMessage)
        {
            // イベントを解除
            CommandProcess.UnregisterHandlerOnCommandResponse(OnCommandResponseRef);

            // 即時でアプリケーションに制御を戻す
            if (success == false) {
                NotifyCommandTerminated(success, errorMessage);
                return;
            }

            // TODO: 仮の実装です。
            NotifyCommandTerminated(success, AppCommon.MSG_NONE);
        }
    }
}
