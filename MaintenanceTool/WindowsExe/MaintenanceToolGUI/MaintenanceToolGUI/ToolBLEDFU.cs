using MaintenanceToolCommon;

namespace MaintenanceToolGUI
{
    class ToolBLEDFU
    {
        // 更新対象アプリケーション＝version 0.4.0
        public const int DFU_UPD_TARGET_APP_VERSION = 400;

        // 画面の参照を保持
        private MainForm mainForm;

        // 処理クラスの参照を保持
        private BLEMain bleMain;

        // 更新イメージクラス
        private ToolBLEDFUImage toolBLEDFUImage;

        // 更新イメージファイル名から取得したバージョン
        public string UpdateVersion { get; set; }

        // 認証器からBLE経由で取得したバージョン
        public string CurrentVersion { get; set; }
        public string CurrentBoardname { get; set; }

        // バージョン更新判定フラグ
        private bool NeedCompareUpdateVersion;

        public ToolBLEDFU(MainForm f, BLEMain b)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // BLE処理クラスの参照を保持
            bleMain = b;

            // 更新イメージクラスを初期化
            toolBLEDFUImage = new ToolBLEDFUImage();

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

            // TODO: 仮の実装です。
            AppCommon.OutputLogDebug(string.Format(
                "ToolBLEDFU: CurrentVersion={0}, CurrentBoardname={1}", CurrentVersion, CurrentBoardname));
            NotifyCancel();
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
                // dfuProcessingForm.NotifyTerminateDFUProcess(CompareUpdateVersion());

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

        //
        // メッセージボックス
        //
        void ShowWarningMessage(string captionText, string messageText)
        {
            FormUtil.ShowWarningMessage(mainForm, captionText, messageText);
        }
    }
}
