using static MaintenanceToolApp.DFU.DFUParameter;

namespace MaintenanceToolApp.DFU
{
    internal class DFUTransferProcess
    {
        // このクラスのインスタンス
        private static readonly DFUTransferProcess Instance = new DFUTransferProcess();

        // BLE SMPサービスの参照を保持（インスタンス生成は１度だけ行われる）
        private static readonly BLESMPService SMPService = new BLESMPService();

        // 上位クラス／パラメーターの参照を保持
        private DFUProcess DFUProcess = null!;
        private DFUParameter Parameter = null!;

        public static void InvokeTransferProcess(DFUProcess process, DFUParameter parameter)
        {
            // 上位クラス／パラメーターの参照を保持
            Instance.DFUProcess = process;
            Instance.Parameter = parameter;

            // 転送処理を起動
            Instance.InvokeDFUProcess();
        }

        //
        // 内部処理
        //
        private void InvokeDFUProcess()
        {
            // ステータスを更新
            Parameter.Status = DFUStatus.UploadProcess;

            // DFU処理開始時の画面処理
            int maximum = 100 + DFUProcessConst.DFU_WAITING_SEC_ESTIMATED;
            DFUProcess.NotifyDFUProcessStarting(maximum);

            // DFU本処理を開始（処理の終了は、処理進捗画面に通知される）
            // SMPサービスに接続
            SMPService.ConnectBLESMPService(OnConnectedToSMPService);
        }

        private void OnConnectedToSMPService(bool success)
        {
            // 転送処理を開始する
            DoTransferProcess();
        }

        private void DoTransferProcess()
        {
            // DFU実行開始を通知
            DFUProcess.NotifyDFUProgress(AppCommon.MSG_DFU_PROCESS_TRANSFER_IMAGE, 0);

            //
            // TODO: 仮の実装です。
            //
            System.Threading.Thread.Sleep(2000);

            DFUProcess.NotifyDFUTransferring(true);
            for (int percentage = 0; Parameter.Status == DFUStatus.UploadProcess && percentage < 100; percentage++) {
                string progressMessage = string.Format(AppCommon.MSG_DFU_PROCESS_TRANSFER_IMAGE_FORMAT, percentage);
                DFUProcess.NotifyDFUProgress(progressMessage, percentage);
                System.Threading.Thread.Sleep(100);
            }
            DFUProcess.NotifyDFUTransferring(false);

            if (Parameter.Status == DFUStatus.Canceled) {
                DFUProcess.OnTerminatedTransferProcess(false);
            } else {
                DFUProcess.OnTerminatedTransferProcess(true);
            }
        }
    }
}
