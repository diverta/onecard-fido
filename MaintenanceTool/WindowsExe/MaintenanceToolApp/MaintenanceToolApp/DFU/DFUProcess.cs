using MaintenanceToolApp.CommonProcess;
using MaintenanceToolApp.CommonWindow;
using System;
using System.Threading.Tasks;
using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.DFU.DFUParameter;

namespace MaintenanceToolApp.DFU
{
    public class DFUParameter
    {
        public enum DFUStatus
        {
            None = 0,
            GetCurrentVersion,
            UploadProcess,
            Canceled,
            ResetDone,
            WaitForBoot,
            CheckUpdateVersion,
        };

        public VersionInfoData CurrentVersionInfo { get; set; }
        public DFUImageData UpdateImageData { get; set; }

        // 処理ステータス
        public DFUStatus Status { get; set; }

        // 処理結果
        public bool Success { get; set; }

        // 処理結果（エラーメッセージ）
        public string ErrorMessage { get; set; }

        public DFUParameter()
        {
            CurrentVersionInfo = null!;
            UpdateImageData = null!;
            Status = DFUStatus.None;
            Success = false;
            ErrorMessage = AppCommon.MSG_NONE;
        }

        public override string ToString()
        {
            return string.Format("CurrentVersionInfo:FW_REV={0}, HW_REV={1} UpdateVersion:{2}",
                CurrentVersionInfo.FWRev, CurrentVersionInfo.HWRev, UpdateImageData.UpdateVersion);
        }
    }

    public class DFUProcessConst
    {
        // 更新対象アプリケーション＝version 0.4.0
        public const int DFU_UPD_TARGET_APP_VERSION = 400;

        // イメージ反映所要時間（秒）
        public const int DFU_WAITING_SEC_ESTIMATED = 25;
    }

    public class DFUProcess
    {
        // 処理実行のためのプロパティー
        private DFUParameter Parameter = new DFUParameter();

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow;

        // 処理進捗画面の参照を保持
        private DFUProcessingWindow ProcessingWindow = null!;

        // BLE SMPサービスの参照を保持（インスタンス生成は１度だけ行われる）
        private static readonly BLESMPService SMPService = new BLESMPService();

        private DFUProcess(Window parentWindowRef)
        {
            // 親ウィンドウの参照を保持
            ParentWindow = parentWindowRef;
        }

        private void DoProcess()
        {
            // DFU主処理を起動
            Task task = Task.Run(() => {
                InvokeDFUProcess();
            });

            // 処理進捗画面を表示
            ProcessingWindow = new DFUProcessingWindow();
            ProcessingWindow.RegisterHandlerOnCanceledTransferByUser(new DFUProcessingWindow.HandlerOnCanceledTransferByUser(OnCanceledTransferByUser));
            if (ProcessingWindow.OpenForm(ParentWindow) == false) {
                // Cancelボタンクリック時は、メッセージをポップアップ表示したのち、画面に制御を戻す
                DialogUtil.ShowWarningMessage(ParentWindow, AppCommon.PROCESS_NAME_BLE_DFU, AppCommon.MSG_DFU_IMAGE_TRANSFER_CANCELED);
                CommandProcess.NotifyCommandTerminated(AppCommon.PROCESS_NAME_NONE, AppCommon.MSG_NONE, true, ParentWindow);
                return;
            }

            // メイン画面に制御を移す
            CommandProcess.NotifyCommandTerminated(AppCommon.PROCESS_NAME_BLE_DFU, Parameter.ErrorMessage, Parameter.Success, ParentWindow);
        }

        //
        // DFU主処理
        //
        private void InvokeDFUProcess()
        {
            // ステータスを更新
            Parameter.Status = DFUStatus.UploadProcess;

            // DFU処理開始時の画面処理
            int maximum = 100 + DFUProcessConst.DFU_WAITING_SEC_ESTIMATED;
            NotifyDFUProcessStarting(maximum);

            // DFU本処理を開始（処理の終了は、処理進捗画面に通知される）
            // SMPサービスに接続
            SMPService.ConnectBLESMPService(OnConnectedToSMPService);
        }

        private void OnConnectedToSMPService(bool success)
        {
            // DFU実行開始を通知
            NotifyDFUProgress(AppCommon.MSG_DFU_PROCESS_TRANSFER_IMAGE, 0);

            //
            // TODO: 仮の実装です。
            //
            System.Threading.Thread.Sleep(2000);

            NotifyDFUTransferring(true);
            for (int percentage = 0; Parameter.Status == DFUStatus.UploadProcess && percentage < 100; percentage++) {
                string progressMessage = string.Format(AppCommon.MSG_DFU_PROCESS_TRANSFER_IMAGE_FORMAT, percentage);
                NotifyDFUProgress(progressMessage, percentage);
                System.Threading.Thread.Sleep(100);
            }
            NotifyDFUTransferring(false);

            if (Parameter.Status == DFUStatus.Canceled) {
                OnTerminatedTransferProcess(false);
            } else {
                OnTerminatedTransferProcess(success);
            }
        }

        private void OnCanceledTransferByUser()
        {
            // 処理進捗画面のCancelボタンがクリックされた場合
            // メッセージ文言を画面とログに出力
            NotifyDFUInfoMessage(AppCommon.MSG_DFU_IMAGE_TRANSFER_CANCELED);

            // ステータスを更新（処理キャンセル）
            Parameter.Status = DFUStatus.Canceled;
        }

        private void OnTerminatedTransferProcess(bool success)
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

            // バージョン情報照会処理に遷移
            // TODO: 仮の実装です。
            Parameter.Success = true;
            NotifyDFUProcessTerminated();
        }

        //
        // 画面に対する処理
        //
        private void NotifyDFUProcessStarting(int maximum)
        {
            Application.Current.Dispatcher.Invoke(new Action(() => {
                // 処理進捗画面にDFU処理開始を通知
                ProcessingWindow.NotifyStartDFUProcess(maximum);
                ProcessingWindow.NotifyDFUProcess(AppCommon.MSG_DFU_PRE_PROCESS, 0);

                // メイン画面に開始メッセージを表示
                CommandProcess.NotifyCommandStarted(AppCommon.PROCESS_NAME_BLE_DFU);
            }));
        }

        private void NotifyDFUInfoMessage(string message)
        {
            // メッセージ文言を画面とログに出力
            Application.Current.Dispatcher.Invoke(new Action(() => {
                CommandProcess.NotifyMessageToMainUI(message);
            }));
            AppLogUtil.OutputLogInfo(message);
        }

        private void NotifyDFUTransferring(bool transferring)
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

        private void NotifyDFUProgress(string message, int progressValue)
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

        //
        // 処理開始前の確認
        //
        public static bool ConfirmDoProcess(Window currentWindow)
        {
            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}",
                AppCommon.MSG_PROMPT_START_BLE_DFU_PROCESS,
                AppCommon.MSG_COMMENT_START_BLE_DFU_PROCESS);

            // プロンプトを表示し、Yesの場合だけ処理を続行する
            return DialogUtil.DisplayPromptPopup(currentWindow, AppCommon.MSG_TOOL_TITLE, message);
        }

        //
        // 処理開始の指示
        //
        public static DFUProcess Instance = null!;

        public static void StartDFUProcess(Window ParentWindow)
        {
            // インスタンスを生成
            Instance = new DFUProcess(ParentWindow);
            Instance.StartDFU();
        }

        private void StartDFU()
        {
            // ステータスを更新
            Parameter.Status = DFUStatus.GetCurrentVersion;

            Task task = Task.Run(() => {
                // バージョン情報照会から開始
                VersionInfoProcess process = new VersionInfoProcess();
                process.DoRequestVersionInfo(new VersionInfoProcess.HandlerOnNotifyCommandTerminated(OnReceivedVersionInfo));
            });

            // 進捗画面を表示
            CommonProcessingWindow.OpenForm(ParentWindow);

            // バージョン情報照会失敗時は、以降の処理を実行しない
            if (Parameter.Status == DFUStatus.None) {
                DialogUtil.ShowWarningMessage(ParentWindow, AppCommon.MSG_TOOL_TITLE, AppCommon.MSG_DFU_VERSION_INFO_GET_FAILED);
                return;
            }

            // 更新ファームウェアのバージョンチェック／イメージ情報取得
            string checkErrorCaption;
            string checkErrorMessage;
            DFUImageData imageData;
            if (DFUImage.CheckAndGetUpdateVersion(Parameter.CurrentVersionInfo, out checkErrorCaption, out checkErrorMessage, out imageData) == false) {
                DialogUtil.ShowWarningMessage(ParentWindow, checkErrorCaption, checkErrorMessage);
                return;
            }

            // イメージ情報をパラメーターに設定
            Parameter.UpdateImageData = imageData;

            // ファームウェア更新画面を開き、実行を指示
            bool b = new DFUWindow(Parameter).ShowDialogWithOwner(ParentWindow);
            if (b) {
                // DFU機能を実行
                DoProcess();
            }
        }

        private void OnReceivedVersionInfo(bool success, string errorMessage, VersionInfoData versionInfoData) 
        {
            if (success == false || versionInfoData == null) {
                // バージョン情報照会失敗時はステータスをクリア
                Parameter.Status = DFUStatus.None;

            } else {
                // バージョン情報をパラメーターに設定
                Parameter.CurrentVersionInfo = versionInfoData;
            }

            Application.Current.Dispatcher.Invoke(new Action(() => {
                // 進捗画面を閉じる
                CommonProcessingWindow.NotifyTerminate();
            }));
        }
    }
}
