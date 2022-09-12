using ToolAppCommon;

namespace MaintenanceToolApp.DFU
{
    internal class BLEDFUProcess
    {
        // このクラスのインスタンス
        private static readonly BLEDFUProcess Instance = new BLEDFUProcess();

        // 処理完了時のイベント
        public delegate void HandlerOnProcessTerminated(bool success);
        private event HandlerOnProcessTerminated OnProcessTerminated = null!;

        //
        // 公開用メソッド
        //
        public static void RegisterHandlerOnProcessTerminated(HandlerOnProcessTerminated handler)
        {
            // 処理完了時のイベントを登録
            Instance.OnProcessTerminated += handler;
        }

        public static void UnregisterHandlerOnProcessTerminated(HandlerOnProcessTerminated handler)
        {
            // 処理完了時のイベントを登録解除
            Instance.OnProcessTerminated -= handler;
        }

        public static void PerformDFUProcess()
        {
            Instance.PerformDFU();
        }

        //
        // 内部処理
        //
        private void PerformDFU()
        {
            // TODO: 仮の実装です。
            System.Threading.Thread.Sleep(2000);
            OnProcessTerminated(true);
        }
    }
}
