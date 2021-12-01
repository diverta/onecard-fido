using MaintenanceToolCommon;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public class ToolBLEDFU
    {
        // 更新対象アプリケーション＝version 0.4.0
        public const int DFU_UPD_TARGET_APP_VERSION = 400;
        // イメージ反映所要時間（秒）
        public const int DFU_WAITING_SEC_ESTIMATED = 25;

        // 画面の参照を保持
        private MainForm mainForm;
        private BLEDFUStartForm startForm;
        private BLEDFUProcessingForm processingForm;

        // 処理クラスの参照を保持
        private BLEMain bleMain;

        // 更新イメージクラス
        private ToolBLEDFUImage toolBLEDFUImage;

        // 更新イメージファイル名から取得したバージョン
        public string UpdateVersion { get; set; }

        // 認証器からBLE経由で取得したバージョン
        public string CurrentVersion { get; set; }
        public string CurrentBoardname { get; set; }

        // 転送処理クラス
        private ToolBLEDFUProcess toolDFUProcess;

        // バージョン更新判定フラグ
        private bool NeedCompareUpdateVersion;

        public ToolBLEDFU(MainForm f, BLEMain b)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // BLE処理クラスの参照を保持
            bleMain = b;

            // 処理開始／進捗画面を生成
            startForm = new BLEDFUStartForm();
            processingForm = new BLEDFUProcessingForm();

            // 更新イメージクラスを初期化
            toolBLEDFUImage = new ToolBLEDFUImage();

            // DFU転送処理クラスを初期化
            toolDFUProcess = new ToolBLEDFUProcess(toolBLEDFUImage);

            // バージョン更新判定フラグをリセット
            NeedCompareUpdateVersion = false;
        }

        //
        // メイン画面用のインターフェース
        //
        public void DoCommandBLEDFU()
        {
            // 認証器に導入中のバージョンを照会
            bleMain.DoGetVersionInfoForDFU(this);
        }

        public void ResumeCommandDFU()
        {
            // 基板名に対応するファームウェア更新イメージファイルから、バイナリーイメージを読込
            if (ReadDFUImageFile() == false) {
                NotifyCancel();
                return;
            }

            // バージョンチェックが不正の場合は処理を終了
            if (DfuImageIsAvailable() == false) {
                NotifyCancel();
                return;
            }

            // 処理開始画面を表示
            if (startForm.OpenForm(mainForm, this)) {
                // 処理開始画面でOKクリック-->DFU接続成功の場合、
                // DFU主処理開始
                DoProcessDFU();
            } else {
                // キャンセルボタンがクリックされた場合は
                // メイン画面に通知
                NotifyCancel();
            }
        }

        private void NotifyCancel()
        {
            // メイン画面に制御を戻す
            mainForm.OnDFUCanceled();
        }

        private bool ReadDFUImageFile()
        {
            // 基板名に対応するファームウェア更新イメージファイルから、バイナリーイメージを読込
            if (toolBLEDFUImage.ReadBLEDFUImageFile(CurrentBoardname) == false) {
                ShowWarningMessage(
                    ToolGUICommon.MSG_DFU_IMAGE_NOT_AVAILABLE,
                    ToolGUICommon.MSG_DFU_UPDATE_IMAGE_FILE_NOT_EXIST);
                return false;
            }
            return true;
        }

        private bool DfuImageIsAvailable()
        {
            // ファームウェア更新イメージファイルから、更新バージョンを取得
            UpdateVersion = toolBLEDFUImage.GetUpdateVersionFromDFUImage();

            // 更新イメージファイル名からバージョンが取得できていない場合は利用不可
            if (UpdateVersion.Equals("")) {
                ShowWarningMessage(
                    ToolGUICommon.MSG_DFU_IMAGE_NOT_AVAILABLE,
                    ToolGUICommon.MSG_DFU_UPDATE_VERSION_UNKNOWN);
                return false;
            }

            // BLE経由で認証器の現在バージョンが取得できていない場合は利用不可
            if (CurrentVersion.Equals("")) {
                ShowWarningMessage(
                    ToolGUICommon.MSG_DFU_IMAGE_NOT_AVAILABLE,
                    ToolGUICommon.MSG_DFU_CURRENT_VERSION_UNKNOWN);
                return false;
            }

            // 認証器の現在バージョンが、更新イメージファイルのバージョンより新しい場合は利用不可
            int currentVersionDec = AppCommon.CalculateDecimalVersion(CurrentVersion);
            int updateVersionDec = AppCommon.CalculateDecimalVersion(UpdateVersion);
            if (currentVersionDec > updateVersionDec) {
                string informative = string.Format(ToolGUICommon.MSG_DFU_CURRENT_VERSION_ALREADY_NEW,
                    CurrentVersion, UpdateVersion);
                ShowWarningMessage(
                    ToolGUICommon.MSG_DFU_IMAGE_NOT_AVAILABLE, informative);
                return false;
            }

            // 認証器の現在バージョンが、所定バージョンより古い場合は利用不可
            // （ブートローダーのバージョンが異なるため）
            if (currentVersionDec < DFU_UPD_TARGET_APP_VERSION) {
                string informative = string.Format(ToolGUICommon.MSG_DFU_CURRENT_VERSION_OLD_FIRMWARE, UpdateVersion);
                ShowWarningMessage(ToolGUICommon.MSG_DFU_IMAGE_NOT_AVAILABLE, informative);
                return false;
            }

            return true;
        }

        //
        // 更新イメージファイル転送完了
        // ～バージョン照会処理完了の間に
        // MainFormがタイムアウト検知した場合の処理
        //
        public void DoCommandTimedOut()
        {
            // DFU処理失敗の旨を処理進捗画面に通知
            processingForm.NotifyTerminateDFUProcess(false);
        }

        //
        // バージョン照会処理
        //
        public void NotifyFirmwareVersionResponse(string strFWRev, string strHWRev)
        {
            // 認証器に導入中のバージョン、基板名を保持
            CurrentVersion = strFWRev;
            CurrentBoardname = strHWRev;

            // バージョン更新判定フラグがセットされている場合（ファームウェア反映待ち）
            if (NeedCompareUpdateVersion) {
                // バージョン更新判定フラグをリセット
                NeedCompareUpdateVersion = false;

                // バージョン情報を比較して終了判定
                // --> 判定結果を処理進捗画面に戻す
                processingForm.NotifyTerminateDFUProcess(CompareUpdateVersion());

            } else {
                // 認証器の現在バージョンと基板名が取得できたら、ファームウェア更新画面を表示
                ResumeCommandDFU();
            }
        }

        public void NotifyFirmwareVersionResponseFailed()
        {
            // メッセージを表示し、メイン画面に制御を戻す
            AppCommon.OutputLogError(ToolGUICommon.MSG_DFU_VERSION_INFO_GET_FAILED);
            FormUtil.ShowWarningMessage(mainForm, MainForm.GetMaintenanceToolTitle(), ToolGUICommon.MSG_DFU_VERSION_INFO_GET_FAILED);
            NotifyCancel();
        }

        private bool CompareUpdateVersion()
        {
            // バージョン情報を比較
            bool versionEqual = (CurrentVersion == UpdateVersion);
            if (versionEqual) {
                // バージョンが同じであればDFU処理は正常終了
                AppCommon.OutputLogInfo(string.Format(
                    ToolGUICommon.MSG_DFU_FIRMWARE_VERSION_UPDATED, UpdateVersion));

            } else {
                // バージョンが同じでなければ異常終了
                AppCommon.OutputLogError(ToolGUICommon.MSG_DFU_FIRMWARE_VERSION_UPDATED_FAILED);
            }

            // メイン画面に制御を戻す
            return versionEqual;
        }

        // 
        // DFU主処理
        // 
        private void DoProcessDFU()
        {
            // DFU主処理を起動
            Task task = Task.Run(() => {
                InvokeDFUProcess();
            });

            // 処理進捗画面を表示
            DialogResult ret = processingForm.OpenForm(mainForm);
            if (ret == DialogResult.Cancel) {
                NotifyCancel();
                return;
            }

            // 処理結果（成功 or 失敗）をメイン画面に戻す
            mainForm.OnAppMainProcessExited(ret == DialogResult.OK);
        }

        private void InvokeDFUProcess()
        {
            // 処理進捗画面にDFU処理開始を通知
            int maximum = 100 + DFU_WAITING_SEC_ESTIMATED;
            processingForm.NotifyStartDFUProcess(maximum);
            processingForm.NotifyDFUProcess(ToolGUICommon.MSG_DFU_PROCESS_TRANSFER_IMAGE, 0);

            // メイン画面に開始メッセージを表示／処理タイムアウト監視を開始
            mainForm.OnDFUStarted();

            // DFU主処理を開始
            toolDFUProcess.PerformDFU(this);
        }

        public void DFUProcessTerminated(bool success)
        {
            if (success) {
                // 処理進捗画面に通知
                processingForm.NotifyDFUProcess(ToolGUICommon.MSG_DFU_PROCESS_WAITING_UPDATE, 100);

                // DFU転送成功時は、バージョン更新判定フラグをセット
                NeedCompareUpdateVersion = true;

            } else {
                // DFU転送失敗時は処理進捗画面に制御を戻す
                processingForm.NotifyTerminateDFUProcess(success);
            }
        }

        //
        // メッセージボックス
        //
        void ShowWarningMessage(string captionText, string messageText)
        {
            FormUtil.ShowWarningMessage(mainForm, captionText, messageText);
        }
    }
}
