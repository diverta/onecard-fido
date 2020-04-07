﻿using MaintenanceToolCommon;

namespace MaintenanceToolGUI
{
    public class ToolDFU
    {
        // 画面の参照を保持
        private MainForm mainForm;
        private DFUStartForm dfuStartForm;
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

        // ブートローダーモード遷移判定フラグ
        private bool NeedCheckBootloaderMode;

        // バージョン更新判定フラグ
        private bool NeedCompareUpdateVersion;

        // DFUで使用する各種パラメーター
        private int MTU;
        private int MaxCreateSize;

        public ToolDFU(MainForm f, HIDMain h)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // HID処理クラスの参照を保持
            hidMain = h;

            // HID処理クラスに、本クラスの参照を設定
            hidMain.ToolDFURef = this;

            // DFUデバイスクラスを初期化
            dfuDevice = new DFUDevice(this);

            // イベントの登録
            dfuDevice.DFUConnectionEstablishedEvent += new DFUDevice.DFUConnectionEstablishedEventHandler(DFUConnectionEstablished);

            // 処理開始／進捗画面を生成
            dfuStartForm = new DFUStartForm(this);
            dfuProcessingForm = new DFUProcessingForm(this);

            // 更新イメージクラスを初期化
            toolDFUImage = new ToolDFUImage();

            // ファームウェア更新イメージファイルから、更新バージョンを取得
            UpdateVersion = toolDFUImage.GetUpdateVersionFromDFUImage();

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
            // バージョンチェックが不正の場合は処理を終了
            if (DfuImageIsAvailable() == false) {
                NotifyCancel();
                return;
            }

            // 処理開始画面を表示
            if (dfuStartForm.OpenForm()) {
                // 処理開始画面でOKクリック-->DFU接続成功の場合、
                // DFU主処理開始
                InvokeDFUProcess();
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

            // DFU処理を開始するかどうかのプロンプトを表示
            string message = string.Format("{0}\n\n{1}",
                ToolGUICommon.MSG_COMMENT_START_DFU_PROCESS,
                ToolGUICommon.MSG_PROMPT_START_DFU_PROCESS
                );
            if (FormUtil.DisplayPromptPopup(message) == false) {
                NotifyCancel();
                return;
            }

            // ファームウェア新規導入処理を開始する
            // TODO: 下記は仮コードです。
            NotifyCancel();
        }

        private void NotifyCancel()
        {
            // メイン画面に制御を戻す
            mainForm.OnDFUCanceled();
        }

        private bool DfuImageIsAvailable()
        {
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
                return true;
            }
            return false;
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
        // バージョン照会処理
        //
        public void OnUSBDeviceArrival()
        {
            // 認証器に導入中のバージョンをクリア
            CurrentVersion = "";

            // 認証器に導入中のバージョンを照会
            hidMain.DoGetVersionInfoForDFU();
        }

        public void NotifyFirmwareVersionResponse(string strFWRev)
        {
            // 認証器に導入中のバージョンを保持
            CurrentVersion = strFWRev;

            // バージョン更新判定フラグがセットされている場合（ファームウェア反映待ち）
            if (NeedCompareUpdateVersion) {
                // バージョン更新判定フラグをリセット
                NeedCompareUpdateVersion = false;

                // バージョン情報を比較して終了判定
                // TODO: 後日実装予定です。
            }
        }

        //
        // DFU対象デバイス接続処理
        //
        private void EstablishDFUConnection()
        {
            // DFU対象デバイスに接続（USB CDC ACM接続）
            dfuDevice.SearchACMDevicePath();
        }

        private void DFUConnectionEstablished(bool success)
        {
            // 処理開始画面に制御を戻す
            dfuStartForm.OnChangeToBootloaderMode(success, 
                MainForm.GetMaintenanceToolTitle(), 
                ToolGUICommon.MSG_DFU_TARGET_NOT_CONNECTED);
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
        private void InvokeDFUProcess()
        {
            // メイン画面に主処理開始を通知
            mainForm.OnDFUStarted();

            // DFU対象デバイスの通知設定
            SendSetReceiptRequest();
        }

        private void SendSetReceiptRequest()
        {
            // SET RECEIPTコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                NRFDfuConst.NRF_DFU_OP_RECEIPT_NOTIF_SET, 0x00, 0x00, NRFDfuConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (dfuDevice.SendDFURequest(b) == false) {
                TerminateDFUProcess(false);
            }
        }

        private void ReceiveSetReceiptRequest(byte[] response)
        {
            // レスポンスがNGの場合は処理終了
            if (AssertDFUResponseSuccess(response) == false) {
                TerminateDFUProcess(false);
            }

            // DFU対象デバイスからMTUを取得
            SendGetMtuRequest();
        }

        private void SendGetMtuRequest()
        {
            // GET MTUコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                NRFDfuConst.NRF_DFU_OP_MTU_GET, NRFDfuConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (dfuDevice.SendDFURequest(b) == false) {
                TerminateDFUProcess(false);
            }
        }

        private void ReceiveGetMtuRequest(byte[] response)
        {
            // レスポンスがNGの場合は処理終了
            if (AssertDFUResponseSuccess(response) == false) {
                TerminateDFUProcess(false);
            }

            // レスポンスからMTUを取得（4〜5バイト目、リトルエンディアン）
            MTU = AppCommon.ToInt16(response, 3, false);
            AppCommon.OutputLogError(string.Format("TooDFU.ReceiveGetMtuRequest: MTU={0}", MTU));

            // DATイメージ転送処理の開始
            // １回あたりの送信データ最大長を取得
            SendSelectObjectRequest(NRFDfuConst.NRF_DFU_BYTE_OBJ_INIT_CMD);
        }

        private void SendSelectObjectRequest(byte objectType)
        {
            // SELECT OBJECTコマンドを生成（DFUリクエスト）
            byte[] b = new byte[] {
                NRFDfuConst.NRF_DFU_OP_OBJECT_SELECT, objectType, NRFDfuConst.NRF_DFU_BYTE_EOM };

            // DFUリクエストを送信
            if (dfuDevice.SendDFURequest(b) == false) {
                TerminateDFUProcess(false);
            }
        }

        private void ReceiveSelectObjectRequest(byte[] response)
        {
            // レスポンスがNGの場合は処理終了
            if (AssertDFUResponseSuccess(response) == false) {
                TerminateDFUProcess(false);
            }

            // レスポンスから種別を取得（3バイト目）
            byte objectType = response[2];

            // レスポンスからMaxCreateSizeを取得（4〜7バイト目、リトルエンディアン）
            MaxCreateSize = AppCommon.ToInt32(response, 3, false);
            AppCommon.OutputLogError(string.Format("TooDFU.ReceiveGetMtuRequest: ObjectType={0}, MaxCreateSize={1}",
                objectType, MaxCreateSize));

            // これは仮の処理です。
            TerminateDFUProcess(true);
        }

        private void TerminateDFUProcess(bool success)
        {
            // DFUデバイスから切断
            dfuDevice.CloseDFUDevice();

            // メイン画面に制御を戻す
            mainForm.OnAppMainProcessExited(success);
        }

        //
        // DFUレスポンス受信時の処理
        //
        public void OnReceiveDFUResponse(bool success, byte[] response)
        {
            // 失敗時はメイン画面に制御を戻す
            if (success == false) {
                TerminateDFUProcess(false);
                return;
            }

            // レスポンスの２バイト目（コマンドバイト）で処理分岐
            byte cmd = response[1];
            switch (cmd) {
            case NRFDfuConst.NRF_DFU_OP_RECEIPT_NOTIF_SET:
                ReceiveSetReceiptRequest(response);
                break;
            case NRFDfuConst.NRF_DFU_OP_MTU_GET:
                ReceiveGetMtuRequest(response);
                break;
            case NRFDfuConst.NRF_DFU_OP_OBJECT_SELECT:
                ReceiveSelectObjectRequest(response);
                break;
            default:
                break;
            }
        }

        private bool AssertDFUResponseSuccess(byte[] response)
        {
            // レスポンスを検証
            if (response == null || response.Length == 0) {
                return false;
            }

            // ステータスコードを参照し、処理が成功したかどうかを判定
            return (response[2] == NRFDfuConst.NRF_DFU_BYTE_RESP_SUCCESS);
        }
    }
}
