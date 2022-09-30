using MaintenanceToolApp.CommonProcess;
using System;
using System.Threading.Tasks;
using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.DFU.DFUParameter;

namespace MaintenanceToolApp.DFU
{
    internal class BLEDFUProcess
    {
        // 処理実行のためのプロパティー
        private DFUParameter Parameter = null!;

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow;

        // 処理進捗画面の参照を保持
        private DFUProcessingWindow ProcessingWindow = null!;

        public BLEDFUProcess(Window parentWindowRef, DFUParameter parameterRef)
        {
            // 親ウィンドウの参照を保持
            ParentWindow = parentWindowRef;

            // パラメーターの参照を保持
            Parameter = parameterRef;
        }

        public void StartUSBDFU()
        {
            // DFU転送処理を起動
            Task task = Task.Run(() => {
                BLEDFUTransferProcess.InvokeTransferProcess(this, Parameter);
            });

            // 処理進捗画面を表示
            ProcessingWindow = new DFUProcessingWindow();
            ProcessingWindow.RegisterHandlerOnCanceledTransferByUser(new DFUProcessingWindow.HandlerOnCanceledTransferByUser(OnCanceledTransferByUser));
            if (ProcessingWindow.OpenForm(ParentWindow) == false) {
                // Cancelボタンクリック時は、メッセージをポップアップ表示したのち、画面に制御を戻す
                DialogUtil.ShowWarningMessage(ParentWindow, AppCommon.PROCESS_NAME_BLE_DFU, AppCommon.MSG_DFU_IMAGE_TRANSFER_CANCELED);
                CommandProcess.NotifyCommandTerminated(AppCommon.PROCESS_NAME_NONE, AppCommon.MSG_NONE, true, ParentWindow);

            } else {
                // メイン画面に制御を移す
                CommandProcess.NotifyCommandTerminated(AppCommon.PROCESS_NAME_BLE_DFU, Parameter.ErrorMessage, Parameter.Success, ParentWindow);
            }

            // ステータスを更新
            Parameter.Status = DFUStatus.None;
        }

        //
        // DFU転送処理中のコールバック
        //
        private void OnCanceledTransferByUser()
        {
            // 処理進捗画面のCancelボタンがクリックされた場合
            // メッセージ文言を画面とログに出力
            NotifyDFUInfoMessage(AppCommon.MSG_DFU_IMAGE_TRANSFER_CANCELED);

            // ステータスを更新（処理キャンセル）
            Parameter.Status = DFUStatus.Canceled;
        }

        public void OnTerminatedTransferProcess(bool success)
        {
            if (Parameter.Status == DFUStatus.Canceled) {
                // 転送が中止された旨を、処理進捗画面に通知
                NotifyCancelDFUProcess();
                return;
            }

            if (success) {
                // ステータスを更新（DFU反映待ち）
                Parameter.Status = DFUStatus.WaitForBoot;

                // DFU反映待ち処理を起動
                PerformDFUUpdateMonitor();

            } else {
                // DFU転送失敗時は処理進捗画面に制御を戻す
                Parameter.Success = false;
                NotifyDFUProcessTerminated();
            }
        }

        // 
        // DFU反映待ち処理
        // 
        private void PerformDFUUpdateMonitor()
        {
            // 処理進捗画面に通知
            NotifyDFUProgress(AppCommon.MSG_DFU_PROCESS_WAITING_UPDATE, 100);

            // 反映待ち（リセットによるファームウェア再始動完了まで待機）
            for (int i = 0; i < DFUProcessConst.DFU_WAITING_SEC_ESTIMATED; i++) {
                // 処理進捗画面に通知
                NotifyDFUProgress(AppCommon.MSG_DFU_PROCESS_WAITING_UPDATE, 100 + i);
                System.Threading.Thread.Sleep(1000);
            }

            // 処理進捗画面に通知
            NotifyDFUProgress(AppCommon.MSG_DFU_PROCESS_CONFIRM_VERSION, 100 + DFUProcessConst.DFU_WAITING_SEC_ESTIMATED);

            // ステータスを更新（バージョン更新判定）
            Parameter.Status = DFUStatus.CheckUpdateVersion;

            // バージョン情報照会処理に遷移
            VersionInfoProcess process = new VersionInfoProcess();
            process.DoRequestVersionInfo(Parameter.Transport, new VersionInfoProcess.HandlerOnNotifyCommandTerminated(OnReceivedUpdateVersionInfo));
        }

        private void OnReceivedUpdateVersionInfo(bool success, string errorMessage, VersionInfoData versionInfoData)
        {
            if (success == false || versionInfoData == null) {
                // バージョン情報照会失敗時は終了
                Parameter.ErrorMessage = errorMessage;
                AppLogUtil.OutputLogError(Parameter.ErrorMessage);

                Parameter.Success = false;
                NotifyDFUProcessTerminated();
                return;
            }

            // バージョン情報を比較して終了判定
            // --> 判定結果をメイン画面に戻す
            Parameter.Success = CompareUpdateVersion(versionInfoData);
            NotifyDFUProcessTerminated();
        }

        private bool CompareUpdateVersion(VersionInfoData versionInfoData)
        {
            // バージョン情報を比較
            string CurrentVersion = versionInfoData.FWRev;
            string UpdateVersion = Parameter.UpdateImageData.UpdateVersion;
            bool versionEqual = (CurrentVersion == UpdateVersion);
            if (versionEqual) {
                // バージョンが同じであればDFU処理は正常終了
                NotifyDFUInfoMessage(string.Format(AppCommon.MSG_DFU_FIRMWARE_VERSION_UPDATED, UpdateVersion));

            } else {
                // バージョンが同じでなければ異常終了
                Parameter.ErrorMessage = string.Format(AppCommon.MSG_DFU_FIRMWARE_VERSION_UPDATED_FAILED, UpdateVersion);
                AppLogUtil.OutputLogError(Parameter.ErrorMessage);
            }

            // 比較結果を戻す
            return versionEqual;
        }

        //
        // 画面に対する処理
        //
        public void NotifyDFUProcessStarting(int maximum)
        {
            Application.Current.Dispatcher.Invoke(new Action(() => {
                // 処理進捗画面にDFU処理開始を通知
                ProcessingWindow.NotifyStartDFUProcess(maximum);
                ProcessingWindow.NotifyDFUProcess(AppCommon.MSG_DFU_PRE_PROCESS, 0);

                // メイン画面に開始メッセージを表示
                CommandProcess.NotifyCommandStarted(AppCommon.PROCESS_NAME_BLE_DFU);
            }));
        }

        public void NotifyDFUInfoMessage(string message)
        {
            // メッセージ文言を画面とログに出力
            Application.Current.Dispatcher.Invoke(new Action(() => {
                CommandProcess.NotifyMessageToMainUI(message);
            }));
            AppLogUtil.OutputLogInfo(message);
        }

        public void NotifyDFUTransferring(bool transferring)
        {
            // 処理進捗画面のCancelボタンを押下可能／不可能とする
            Application.Current.Dispatcher.Invoke(new Action(() => {
                ProcessingWindow.NotifyCancelable(transferring);
            }));
        }

        public void NotifyCancelDFUProcess()
        {
            Application.Current.Dispatcher.Invoke(new Action(() => {
                ProcessingWindow.NotifyCancelDFUProcess();
            }));
        }

        public void NotifyDFUProgress(string message, int progressValue)
        {
            Application.Current.Dispatcher.Invoke(new Action(() => {
                ProcessingWindow.NotifyDFUProcess(message, progressValue);
            }));
        }

        public void NotifyDFUProcessTerminated()
        {
            Application.Current.Dispatcher.Invoke(new Action(() => {
                ProcessingWindow.NotifyDFUProcessTerminated();
            }));
        }
    }
}
