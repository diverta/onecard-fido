using System;
using ToolGUICommon;

namespace DevelopmentToolGUI
{
    internal static class Const
    {
        // HIDコマンドバイトに関する定義
        public const int HID_CMD_CTAPHID_INIT = 0x86;
        public const int HID_CMD_ERASE_SKEY_CERT = 0xc0;
        public const int HID_CMD_INSTALL_SKEY_CERT = 0xc1;
        public const int HID_CMD_INSTALL_ATTESTATION = 0xc8;
        public const int HID_CMD_BOOTLOADER_MODE = 0xc5;
        public const int HID_CMD_CTAPHID_CBOR = 0x90;
        public const int HID_CMD_UNKNOWN_ERROR = 0xbf;
    }

    public class HIDMain
    {
        // メイン画面の参照を保持
        private MainForm mainForm;
        // HIDデバイス関連
        private HIDProcess hidProcess = new HIDProcess();

        // CTAP2共通処理
        private Ctap2 ctap2;

        // ブロードキャストCIDを保持
        private readonly byte[] CIDBytes = { 0xff, 0xff, 0xff, 0xff};

        // INITコマンドで受信したCIDを保持
        private byte[] ReceivedCID = null;

        // nonceを保持
        private byte[] nonceBytes = new byte[8];

        // リクエストデータ格納領域
        private byte[] RequestData = new byte[1024];

        // 当初リクエストされたHIDコマンドを退避
        private byte requestedCMD;

        // 実行機能を保持
        private AppCommon.RequestType requestType;

        public HIDMain(MainForm f)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // HIDデバイス関連
            // イベントの登録
            hidProcess.MessageTextEvent += new HIDProcess.MessageTextEventHandler(PrintMessageText);
            hidProcess.ReceiveHIDMessageEvent += new HIDProcess.ReceiveHIDMessageEventHandler(ReceiveHIDMessage);
            hidProcess.HIDConnectedEvent += new HIDProcess.HIDConnectedEventHandler(NotifyHIDDetectConnect);

            // FIDOデバイスに接続
            //  ウィンドウのハンドルを引き渡す
            hidProcess.OnFormCreate(mainForm.Handle);

            // CTAP2共通処理に各種参照を引き渡す
            ctap2 = new Ctap2(mainForm, AppCommon.TRANSPORT_HID);
            ctap2.SetHidMain(this);
        }

        public void OnFormDestroy()
        {
            // U2F HIDデバイスを切断
            hidProcess.OnFormDestroy();
        }

        public void OnUSBDeviceArrival()
        {
            // USB HIDデバイスに接続
            hidProcess.OnUSBDeviceArrival();
        }

        public void OnUSBDeviceRemoveComplete()
        {
            hidProcess.OnUSBDeviceRemoveComplete();
        }

        private void ReceiveHIDMessage(byte[] message, int length)
        {
            // HIDデバイスからメッセージ受信時の処理を行う
            switch (hidProcess.receivedCMD) {
            case Const.HID_CMD_CTAPHID_INIT:
                DoResponseTestCtapHidInit(message, length);
                break;
            case Const.HID_CMD_ERASE_SKEY_CERT:
            case Const.HID_CMD_INSTALL_SKEY_CERT:
                // ステータスバイトをチェックし、画面に制御を戻す
                DoResponseMaintSkeyCert(message, length);
                break;
            case Const.HID_CMD_CTAPHID_CBOR:
                ctap2.DoResponseCtapHidCbor(message, length);
                break;
            case Const.HID_CMD_UNKNOWN_ERROR:
                // メイン画面に制御を戻す
                mainForm.OnPrintMessageText(AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                mainForm.OnAppMainProcessExited(false);
                break;
            }
        }

        private void PrintMessageText(string messageText)
        {
            // 画面のテキストエリアにメッセージを表示
            mainForm.OnPrintMessageText(messageText);
        }

        public bool IsUSBDeviceDisconnected()
        {
            // USB HID接続がない場合は true
            if (hidProcess.IsUSBDeviceDisconnected()) {
                return true;
            }
            return false;
        }

        private bool CheckUSBDeviceDisconnected()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (hidProcess.IsUSBDeviceDisconnected()) {
                PrintMessageText(AppCommon.MSG_CMDTST_PROMPT_USB_PORT_SET);
                mainForm.OnAppMainProcessExited(false);
                return true;
            }
            return false;
        }

        public void SendHIDMessage(byte cmd, byte[] message, int length)
        {
            if (ReceivedCID != null) {
                requestedCMD = cmd;
                hidProcess.SendHIDMessage(ReceivedCID, cmd, message, length);
            }
        }

        //
        // CTAP2HID_INITコマンド関連処理
        //
        public void DoRequestCtapHidInit(AppCommon.RequestType t)
        {
            // 実行するコマンドを退避
            requestType = t;

            // 8バイトのランダムデータを生成
            new Random().NextBytes(nonceBytes);

            // INITコマンドを実行し、nonce を送信する
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_CTAPHID_INIT, nonceBytes, nonceBytes.Length);
        }

        private void DoResponseTestCtapHidInit(byte[] message, int length)
        {
            // nonceの一致チェック
            for (int i = 0; i < nonceBytes.Length; i++) {
                if (nonceBytes[i] != message[i]) {
                    // nonceが一致しない場合は異常終了
                    PrintMessageText(AppCommon.MSG_CMDTST_INVALID_NONCE);
                    mainForm.OnAppMainProcessExited(false);
                    return;
                }
            }

            // レスポンスされたCIDを抽出し、クラス内で保持
            ReceivedCID = ExtractReceivedCID(message);

            // INITコマンドの後続処理判定
            DoResponseCtapHidInit(message, length);
        }

        public void DoResponseCtapHidInit(byte[] message, int length)
        {
            switch (requestType) {
            case AppCommon.RequestType.EraseSkeyCert:
                DoRequestEraseSkeyCert();
                break;
            case AppCommon.RequestType.InstallSkeyCert:
                // 認証器の公開鍵を取得
                ctap2.DoGetKeyAgreement(requestType);
                break;
            default:
                break;
            }
        }

        private byte[] ExtractReceivedCID(byte[] message)
        {
            // メッセージからCIDを抽出して戻す
            byte[] receivedCID = new byte[4];
            for (int j = 0; j < receivedCID.Length; j++) {
                receivedCID[j] = message[8 + j];
            }
            return receivedCID;
        }

        public void DoEraseSkeyCert()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit(AppCommon.RequestType.EraseSkeyCert);
        }

        public void DoRequestEraseSkeyCert()
        {
            // コマンドバイトだけを送信する
            hidProcess.SendHIDMessage(ReceivedCID, Const.HID_CMD_ERASE_SKEY_CERT, RequestData, 0);
        }

        // インストール元の鍵・証明書ファイルパスを保持
        private string skeyFilePathForInstall;
        private string certFilePathForInstall;

        public void DoInstallSkeyCert(string skeyFilePath, string certFilePath)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }

            // インストール元の鍵・証明書ファイルパスを保持
            skeyFilePathForInstall = skeyFilePath;
            certFilePathForInstall = certFilePath;

            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit(AppCommon.RequestType.InstallSkeyCert);
        }

        public void DoRequestInstallSkeyCert(byte[] agreementKeyCBOR)
        {
            AppUtil.OutputLogDebug("DoRequestInstallSkeyCert");
            // CBORレスポンスから、公開鍵を抽出
            InstallSkeyCert installSkeyCert = new InstallSkeyCert();
            if (installSkeyCert.ExtractKeyAgreement(agreementKeyCBOR) == false) {
                mainForm.OnPrintMessageText(AppCommon.MSG_CANNOT_RECV_DEVICE_PUBLIC_KEY);
                mainForm.OnAppMainProcessExited(false);
                return;
            }
            // 秘密鍵をファイルから読込
            if (installSkeyCert.ReadPemFile(skeyFilePathForInstall) == false) {
                mainForm.OnPrintMessageText(AppCommon.MSG_CANNOT_READ_SKEY_PEM_FILE);
                mainForm.OnAppMainProcessExited(false);
                return;
            }
            // 証明書をファイルから読込
            if (installSkeyCert.ReadCertFile(certFilePathForInstall) == false) {
                mainForm.OnPrintMessageText(AppCommon.MSG_CANNOT_READ_CERT_CRT_FILE);
                mainForm.OnAppMainProcessExited(false);
                return;
            }
            // 秘密鍵と証明書の整合性検証を行う
            if (installSkeyCert.ValidateSkeyCert() == false) {
                mainForm.OnPrintMessageText(AppCommon.MSG_INVALID_SKEY_OR_CERT);
                mainForm.OnAppMainProcessExited(false);
                return;
            }
            // 秘密鍵・証明書の内容を暗号化して配列にセットし、HIDデバイスに送信
            byte[] cbor = installSkeyCert.GenerateInstallSkeyCertBytes();
            if (cbor == null) {
                mainForm.OnPrintMessageText(AppCommon.MSG_CANNOT_CRYPTO_SKEY_CERT_DATA);
                mainForm.OnAppMainProcessExited(false);
                return;
            }
            AppUtil.OutputLogDebug("DoRequestInstallSkeyCert hidProcess.SendHIDMessage");
            hidProcess.SendHIDMessage(ReceivedCID, Const.HID_CMD_INSTALL_SKEY_CERT, cbor, cbor.Length);
        }

        private void DoResponseMaintSkeyCert(byte[] message, int length)
        {
            // ステータスバイトをチェック
            bool result = (message[0] == 0x00);
            // 画面に制御を戻す
            mainForm.OnAppMainProcessExited(result);
        }

        //
        // USB接続検知時の処理
        //
        private void NotifyHIDDetectConnect()
        {
        }
    }
}
