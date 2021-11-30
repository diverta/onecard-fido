using MaintenanceToolCommon;

namespace MaintenanceToolGUI
{
    class ToolBLEDFU
    {
        // 画面の参照を保持
        private MainForm mainForm;

        // 処理クラスの参照を保持
        private BLEMain bleMain;

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
    }
}
