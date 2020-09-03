using MaintenanceToolCommon;
using System.Threading.Tasks;

namespace MaintenanceToolGUI
{
    public class ToolDFU
    {
        // 画面の参照を保持
        private MainForm mainForm;
        private DFUStartForm dfuStartForm;
        private DFUNewStartForm dfuNewStartForm;
        private DFUProcessingForm dfuProcessingForm;

        // 処理クラスの参照を保持
        private HIDMain hidMain;
        private DFUDevice dfuDevice;

        // 更新イメージクラス
        private ToolDFUImage toolDFUImage;

        // 更新イメージファイル名から取得したバージョン
        public string UpdateVersion { get; set; }

        // 認証器からHID経由で取得したバージョン
        public string CurrentVersion { get; set; }
        public string CurrentBoardname { get; set; }

        // 転送処理クラス
        private ToolDFUProcess toolDFUProcess;

        // ブートローダーモード遷移判定フラグ
        private bool NeedCheckBootloaderMode;

        // バージョン更新判定フラグ
        private bool NeedCompareUpdateVersion;

        // 処理区分
        private enum ToolDFUCommand
        {
            CommandDFU = 0,
            CommandDFUNew
        };
        private ToolDFUCommand Command;

        public ToolDFU(MainForm f, HIDMain h)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // HID処理クラスの参照を保持
            hidMain = h;

            // HID処理クラスに、本クラスの参照を設定
            hidMain.ToolDFURef = this;

            // DFUデバイスクラスを初期化
            dfuDevice = new DFUDevice();

            // イベントの登録
            dfuDevice.DFUConnectionEstablishedEvent += new DFUDevice.DFUConnectionEstablishedEventHandler(DFUConnectionEstablished);

            // 処理開始／進捗画面を生成
            dfuStartForm = new DFUStartForm(this);
            dfuNewStartForm = new DFUNewStartForm(this);
            dfuProcessingForm = new DFUProcessingForm();

            // 更新イメージクラスを初期化
            toolDFUImage = new ToolDFUImage();

            // DFU転送処理クラスを初期化
            toolDFUProcess = new ToolDFUProcess(dfuDevice, toolDFUImage);

            // イベントの登録
            toolDFUProcess.DFUProcessTerminatedEvent += new ToolDFUProcess.DFUProcessTerminatedEventHandler(DFUProcessTerminated);

            // ブートローダーモード遷移判定フラグをリセット
            NeedCheckBootloaderMode = false;

            // バージョン更新判定フラグをリセット
            NeedCompareUpdateVersion = false;
        }

        //
        // メイン画面用のインターフェース
        //
        public void DoCommandDFU()
        {
            // 認証器に導入中のバージョンを照会
            hidMain.DoGetVersionInfoForDFU();
        }

        public void ResumeCommandDFU()
        {
            // 基板名に対応するファームウェア更新イメージファイルから、バイナリーイメージを読込
            if (ReadDFUImageFile() == false) {
                NotifyCancel();
                return;
            }

            // バージョンチェックが不正の場合は処理を終了
            if (VersionCheckForDFU() == false) {
                NotifyCancel();
                return;
            }
            if (DfuImageIsAvailable() == false) {
                NotifyCancel();
                return;
            }

            // 処理区分を設定
            Command = ToolDFUCommand.CommandDFU;

            // 処理開始画面を表示
            if (dfuStartForm.OpenForm()) {
                // 処理開始画面でOKクリック-->DFU接続成功の場合、
                // DFU主処理開始
                DoProcessDFU();
            } else {
                // キャンセルボタンがクリックされた場合は
                // メイン画面に通知
                NotifyCancel();
            }
        }

        public void DoCommandDFUNew()
        {
            // バージョンチェックが不正の場合は処理を終了
            if (DfuImageIsAvailable() == false) {
                NotifyCancel();
                return;
            }

            // 処理区分を設定
            Command = ToolDFUCommand.CommandDFUNew;

            // 処理開始画面を表示
            if (dfuNewStartForm.OpenForm()) {
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
            if (toolDFUImage.ReadDFUImageFile() == false) {
                FormUtil.ShowWarningMessage(
                    ToolGUICommon.MSG_DFU_IMAGE_NOT_AVAILABLE,
                    ToolGUICommon.MSG_DFU_UPDATE_IMAGE_FILE_NOT_EXIST);
                return false;
            }
            return true;
        }

        private bool DfuImageIsAvailable()
        {
            // ファームウェア更新イメージファイルから、更新バージョンを取得
            UpdateVersion = toolDFUImage.GetUpdateVersionFromDFUImage();

            // 更新イメージファイル名からバージョンが取得できていない場合は利用不可
            if (UpdateVersion.Equals("")) {
                FormUtil.ShowWarningMessage(
                    ToolGUICommon.MSG_DFU_IMAGE_NOT_AVAILABLE,
                    ToolGUICommon.MSG_DFU_UPDATE_VERSION_UNKNOWN);
                return false;
            }
            return true;
        }

        private bool VersionCheckForDFU()
        {
            // HID経由で認証器の現在バージョンが取得できていない場合は利用不可
            if (CurrentVersion.Equals("")) {
                FormUtil.ShowWarningMessage(
                    ToolGUICommon.MSG_DFU_IMAGE_NOT_AVAILABLE,
                    ToolGUICommon.MSG_DFU_CURRENT_VERSION_UNKNOWN);
                return false;
            }
            return true;
        }

        //
        // ブートローダーモード遷移処理
        //
        public void NotifyBootloaderModeResponse(byte receivedCmd, byte[] message)
        {
            if (receivedCmd == Const.HID_CMD_BOOTLOADER_MODE) {
                // ブートローダーモード遷移コマンド成功時は、
                // ブートローダーモード遷移判定フラグをセット
                // HID接続切断 --> OnUSBDeviceRemoveComplete 呼出まで待機
                NeedCheckBootloaderMode = true;

            } else {
                // ブートローダーモード遷移コマンド失敗時は、
                // ブートローダーモード遷移判定フラグをリセット
                NeedCheckBootloaderMode = false;

                // 処理開始画面に制御を戻す
                dfuStartForm.OnChangeToBootloaderMode(false, 
                    ToolGUICommon.MSG_DFU_TARGET_NOT_BOOTLOADER_MODE,
                    ToolGUICommon.MSG_DFU_TARGET_NOT_SECURE_BOOTLOADER);
            }
        }

        public void OnUSBDeviceRemoveComplete()
        {
            // ブートローダーモード遷移判定フラグがセットされている場合（モード遷移完了待ち）
            if (NeedCheckBootloaderMode) {
                // ブートローダーモード遷移判定フラグをリセット
                NeedCheckBootloaderMode = false;

                // DFU対象デバイスへの接続処理を実行
                EstablishDFUConnection();
            }
        }

        //
        // 更新イメージファイル転送完了
        // ～バージョン照会処理完了の間に
        // MainFormがタイムアウト検知した場合の処理
        //
        public void DoCommandTimedOut()
        {
            // DFU処理失敗の旨を処理進捗画面に通知
            dfuProcessingForm.NotifyTerminateDFUProcess(false);
        }

        //
        // バージョン照会処理
        //
        public void OnUSBDeviceArrival()
        {
            // バージョン更新判定フラグがセットされていない場合は終了
            if (NeedCompareUpdateVersion == false) {
                return;
            }

            // 認証器に導入中のバージョン、基板名をクリア
            CurrentVersion = "";
            CurrentBoardname = "";

            // 認証器に導入中のバージョンを照会
            hidMain.DoGetVersionInfoForDFU();
        }

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
                dfuProcessingForm.NotifyTerminateDFUProcess(CompareUpdateVersion());

            } else {
                // 認証器の現在バージョンと基板名が取得できたら、ファームウェア更新画面を表示
                ResumeCommandDFU();
            }
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
        // DFU対象デバイス接続処理
        //
        public void EstablishDFUConnection()
        {
            // DFU対象デバイスに接続（USB CDC ACM接続）
            dfuDevice.SearchACMDevicePath();
        }

        private void DFUConnectionEstablished(bool success)
        {
            if (Command == ToolDFUCommand.CommandDFUNew) {
                // 処理開始画面に制御を戻す
                dfuNewStartForm.OnDFUConnectionEstablished(success,
                    MainForm.GetMaintenanceToolTitle(),
                    ToolGUICommon.MSG_DFU_TARGET_NOT_CONNECTED);

            } else {
                // 処理開始画面に制御を戻す
                dfuStartForm.OnChangeToBootloaderMode(success,
                    MainForm.GetMaintenanceToolTitle(),
                    ToolGUICommon.MSG_DFU_TARGET_NOT_CONNECTED);
            }
        }

        //
        // 処理開始画面用のインターフェース
        //
        public bool ChangeToBootloaderMode()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return false;
            }

            // ブートローダーモード遷移コマンドを実行
            hidMain.DoCommandChangeToBootloaderMode();
            return true;
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
            bool ret = dfuProcessingForm.OpenForm();

            // 処理結果（成功 or 失敗）をメイン画面に戻す
            mainForm.OnAppMainProcessExited(ret);
        }

        private void InvokeDFUProcess()
        {
            // 処理進捗画面にDFU処理開始を通知
            dfuProcessingForm.NotifyStartDFUProcess();
            dfuProcessingForm.NotifyDFUProcess(ToolGUICommon.MSG_DFU_PROCESS_TRANSFER_IMAGE);

            // メイン画面に主処理開始を通知
            mainForm.OnDFUStarted();

            // DFU主処理を開始
            toolDFUProcess.PerformDFU();
        }

        private void DFUProcessTerminated(bool success)
        {
            // DFUデバイスから切断
            dfuDevice.CloseDFUDevice();

            if (success) {
                // 処理進捗画面に通知
                dfuProcessingForm.NotifyDFUProcess(ToolGUICommon.MSG_DFU_PROCESS_WAITING_UPDATE);

                // DFU転送成功時は、バージョン更新判定フラグをセット
                NeedCompareUpdateVersion = true;

            } else {
                // DFU転送失敗時は処理進捗画面に制御を戻す
                dfuProcessingForm.NotifyTerminateDFUProcess(success);
            }
        }
    }
}
