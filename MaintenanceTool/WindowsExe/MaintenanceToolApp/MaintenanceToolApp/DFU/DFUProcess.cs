using MaintenanceToolApp.CommonProcess;
using MaintenanceToolApp.CommonWindow;
using System;
using System.Threading.Tasks;
using System.Windows;
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

        private DFUProcess(Window parentWindowRef, DFUParameter parameterRef)
        {
            // 親ウィンドウの参照を保持
            ParentWindow = parentWindowRef;

            // パラメーターの参照を保持
            Parameter = parameterRef;
        }

        private void DoProcess()
        {
            // USB DFUの場合
            if (Parameter.Transport == Transport.TRANSPORT_CDC_ACM) {
                new USBDFUProcess(ParentWindow, Parameter).StartUSBDFU();
                return;
            }

            // BLE DFUの場合
            if (Parameter.Transport == Transport.TRANSPORT_BLE) {
                new BLEDFUProcess(ParentWindow, Parameter).StartUSBDFU();
                return;
            }
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

        public static void StartDFUProcess(Window ParentWindow, DFUParameter param)
        {
            // インスタンスを生成
            Instance = new DFUProcess(ParentWindow, param);
            Instance.StartDFU();
        }

        private void StartDFU()
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
