using MaintenanceToolApp.CommonProcess;
using MaintenanceToolApp.CommonWindow;
using System;
using System.Threading.Tasks;
using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;
using static MaintenanceToolApp.DFU.DFUParameter;

namespace MaintenanceToolApp.DFU
{
    public class DFUParameter
    {
        public enum DFUStatus
        {
            None = 0,
            GetCurrentVersion,
            ToBootloaderMode,
            UploadProcess,
            Canceled,
            ResetDone,
            WaitForBoot,
            CheckUpdateVersion,
        };

        public VersionInfoData CurrentVersionInfo { get; set; }
        public DFUImageData UpdateImageData { get; set; }

        // 転送区分を保持
        public Transport Transport { get; set; }

        // 転送済みバイト数を保持
        public int ImageBytesSent { get; set; }

        // 処理区分
        public enum BLEDFUCommand
        {
            GetSlotInfo = 0,
            UploadImage,
            ChangeImageUpdateMode,
            ResetApplication,
        };
        public BLEDFUCommand Command { get; set; }

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
            ImageBytesSent = 0;
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

        // イメージ反映モード　true＝テストモード[Swap type: test]、false＝通常モード[Swap type: perm]
        public const bool IMAGE_UPDATE_TEST_MODE = false;

        //
        // nRF52固有対応
        //
        // 更新対象アプリケーション＝version 0.3.0
        public const int DFU_UPD_TARGET_APP_VERSION_FOR_52 = 300;
    }

    public class DFUProcess
    {
        // 処理実行のためのプロパティー
        private DFUParameter Parameter = null!;

        // 親ウィンドウの参照を保持
        private readonly Window ParentWindow;

        public DFUProcess(Window parentWindowRef, DFUParameter parameterRef)
        {
            // 親ウィンドウの参照を保持
            ParentWindow = parentWindowRef;

            // パラメーターの参照を保持
            Parameter = parameterRef;
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
        public void StartDFUProcess()
        {
            // ステータスを更新
            Parameter.Status = DFUStatus.GetCurrentVersion;

            Task task = Task.Run(() => {
                // バージョン情報照会から開始
                VersionInfoProcess process = new VersionInfoProcess();
                process.DoRequestVersionInfo(Parameter.Transport, new VersionInfoProcess.HandlerOnNotifyCommandTerminated(OnReceivedVersionInfo));
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
            if (new DFUWindow(Parameter).ShowDialogWithOwner(ParentWindow)) {
                StartUSBDFU();
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

        //
        // DFU処理開始-->処理進捗画面表示
        //
        // 処理進捗画面の参照を保持
        private DFUProcessingWindow ProcessingWindow = null!;

        public void StartUSBDFU()
        {
            if (Parameter.Transport == Transport.TRANSPORT_CDC_ACM) {
                // USB DFU処理を起動
                Task task = Task.Run(() => {
                    USBDFUTransferProcess.InvokeTransferProcess(this, Parameter);
                });

            } else if (Parameter.Transport == Transport.TRANSPORT_BLE) {
                // BLE DFU処理を起動
                Task task = Task.Run(() => {
                    BLEDFUTransferProcess.InvokeTransferProcess(this, Parameter);
                });

            } else {
                CommandProcess.NotifyCommandTerminated(AppCommon.PROCESS_NAME_NONE, AppCommon.MSG_NONE, true, ParentWindow);
                return;
            }

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

            // 処理進捗画面に制御を戻す
            Parameter.Success = success;
            NotifyDFUProcessTerminated();
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
