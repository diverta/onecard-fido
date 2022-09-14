namespace MaintenanceToolApp.DFU
{
    internal class DFUTransferProcess
    {
        // このクラスのインスタンス
        private static readonly DFUTransferProcess Instance = new DFUTransferProcess();

        // 上位クラス／パラメーターの参照を保持
        private DFUProcess DFUProcess = null!;
        private DFUParameter DFUParameter = null!;

        public void InvokeTransferProcess(DFUProcess process, DFUParameter parameter)
        {
            // 上位クラス／パラメーターの参照を保持
            Instance.DFUProcess = process;
            Instance.DFUParameter = parameter;

            // 転送処理を起動
            Instance.DoTransferProcess();
        }

        //
        // 内部処理
        //
        public void DoTransferProcess()
        {
        }
    }
}
