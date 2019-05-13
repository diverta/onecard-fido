using MaintenanceToolCommon;

namespace MaintenanceToolGUI
{
    internal static class Const
    {
        // HIDフレームに関する定義
        public const int HID_FRAME_LEN = 64;
        public const int HID_INIT_HEADER_LEN = 7;
        public const int HID_CONT_HEADER_LEN = 5;
        // HIDコマンドバイトに関する定義
        public const int HID_CMD_CTAPHID_INIT = 0x86;
        public const int HID_CMD_ERASE_SKEY_CERT = 0xc0;
        public const int HID_CMD_INSTALL_SKEY_CERT = 0xc1;
        public const int HID_CMD_CTAPHID_CBOR = 0x90;
        // サブコマンドバイトに関する定義
        public const byte HID_CBORCMD_NONE = 0x00;
        public const byte HID_CBORCMD_MAKE_CREDENTIAL = 0x01;
        public const byte HID_CBORCMD_GET_ASSERTION = 0x02;
        public const byte HID_CBORCMD_CLIENT_PIN = 0x06;
        public const byte HID_CBORCMD_AUTH_RESET = 0x07;
        public const byte HID_SUBCMD_CLIENT_PIN_GET_AGREEMENT = 0x02;
        public const byte HID_SUBCMD_CLIENT_PIN_SET = 0x03;
        public const byte HID_SUBCMD_CLIENT_PIN_CHANGE = 0x04;
        public const byte HID_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN = 0x05;
    }

    internal class HIDMain
    {
        // メイン画面の参照を保持
        private MainForm mainForm;
        // HIDデバイス関連
        private HIDProcess hidProcess = new HIDProcess();

        // ブロードキャストCID、nonceを保持
        private readonly byte[] CIDBytes = { 0xff, 0xff, 0xff, 0xff};
        private readonly byte[] nonceBytes = {0x71, 0xcb, 0x1c, 0x3b, 0x10, 0x8e, 0xc9, 0x24};

        // リクエストデータ格納領域
        private byte[] RequestData = new byte[1024];

        // 実行中のサブコマンドを保持
        private byte cborCommand;
        private byte subCommand;

        // PINコード設定処理の実行引数を退避
        private string clientPinNew;
        private string clientPinOld;

        // ヘルスチェック処理の実行引数を退避
        private string clientPin;

        // 共通鍵を退避
        //   getPinToken時に生成した共通鍵を、
        //   makeCredential、getAssertion実行時まで保持しておく
        private byte[] SharedSecretKey = null;

        // ユーザー登録情報を退避
        //   makeCredential時に受信したユーザー登録情報を、
        //   getAssertion実行時まで保持しておく
        private MakeCredentialResponse MakeCredentialRes = null;

        // 実行機能を保持
        private enum HIDRequestType
        {
            None = 0,
            ClientPinSet,
            TestCtapHidInit,
            TestMakeCredential,
            TestGetAssertion,
            AuthReset
        };
        private HIDRequestType requestType;

        public HIDMain(MainForm f)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // HIDデバイス関連
            // イベントの登録
            hidProcess.MessageTextEvent += new HIDProcess.MessageTextEventHandler(PrintMessageText);
            hidProcess.ReceiveHIDMessageEvent += new HIDProcess.ReceiveHIDMessageEventHandler(ReceiveHIDMessage);

            // FIDOデバイスに接続
            //  ウィンドウのハンドルを引き渡す
            hidProcess.OnFormCreate(mainForm.Handle);
        }

        public void OnFormDestroy()
        {
            // U2F HIDデバイスを切断
            hidProcess.OnFormDestroy();
        }

        public void OnUSBDeviceArrival()
        {
            hidProcess.OnUSBDeviceArrival();
        }

        public void OnUSBDeviceRemoveComplete()
        {
            hidProcess.OnUSBDeviceRemoveComplete();
        }

        private void ReceiveHIDMessage(byte[] message, int length)
        {
            // HIDデバイスからメッセージ受信時の処理を行う
            switch(hidProcess.receivedCMD) {
            case Const.HID_CMD_CTAPHID_INIT:
                DoResponseTestCtapHidInit(message, length);
                break;
            case Const.HID_CMD_ERASE_SKEY_CERT:
            case Const.HID_CMD_INSTALL_SKEY_CERT:
                DoResponseMaintSkeyCert(message, length);
                break;
            case Const.HID_CMD_CTAPHID_CBOR:
                DoResponseCtapHidCbor(message, length);
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

        public void DoTestCtapHidInit()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // 実行するコマンドを退避
            requestType = HIDRequestType.TestCtapHidInit;
            cborCommand = Const.HID_CBORCMD_NONE;
            // nonce を送信する
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_CTAPHID_INIT, nonceBytes, nonceBytes.Length);
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

        private void DoResponseTestCtapHidInit(byte[] message, int length)
        {
            // nonceの一致チェック
            bool result = true;
            for (int i = 0; i < nonceBytes.Length; i++) {
                if (nonceBytes[i] != message[i]) {
                    result = false;
                    break;
                }
            }
            if (cborCommand == Const.HID_CBORCMD_CLIENT_PIN) {
                // レスポンスされたCIDを抽出し、PIN設定処理を続行
                DoGetKeyAgreement(ExtractReceivedCID(message));
            } else if (cborCommand == Const.HID_CBORCMD_AUTH_RESET) {
                // レスポンスされたCIDを抽出し、Reset処理を続行
                DoRequestAuthReset(ExtractReceivedCID(message));
            } else { 
                // 画面に制御を戻す
                mainForm.OnAppMainProcessExited(result);
            }
        }

        public void DoEraseSkeyCert()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // コマンドバイトだけを送信する
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_ERASE_SKEY_CERT, RequestData, 0);
        }

        public void DoInstallSkeyCert(string skeyFilePath, string certFilePath)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // 秘密鍵をファイルから読込
            InstallSkeyCert installSkeyCert = new InstallSkeyCert();
            if (installSkeyCert.ReadPemFile(skeyFilePath) == false) {
                mainForm.OnAppMainProcessExited(false);
                return;
            }
            // 証明書をファイルから読込
            if (installSkeyCert.ReadCertFile(certFilePath) == false) {
                mainForm.OnAppMainProcessExited(false);
                return;
            }
            // 秘密鍵・証明書の内容を配列にセットし、HIDデバイスに送信
            int RequestDataSize = installSkeyCert.GenerateInstallSkeyCertBytes(RequestData);
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_INSTALL_SKEY_CERT, RequestData, RequestDataSize);
        }

        private void DoResponseMaintSkeyCert(byte[] message, int length)
        {
            // ステータスバイトをチェック
            bool result = (message[0] == 0x00);
            // 画面に制御を戻す
            mainForm.OnAppMainProcessExited(result);
        }

        public void DoAuthReset()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // 実行するコマンドを退避
            requestType = HIDRequestType.AuthReset;
            cborCommand = Const.HID_CBORCMD_AUTH_RESET;
            // nonce を送信する
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_CTAPHID_INIT, nonceBytes, nonceBytes.Length);
        }

        public void DoRequestAuthReset(byte[] receivedCID)
        {
            // リクエスト転送の前に、
            // 基板上ののMAIN SWを押してもらうように促す
            // メッセージを画面表示
            mainForm.OnPrintMessageText(ToolGUICommon.MSG_CLEAR_PIN_CODE_COMMENT1);
            mainForm.OnPrintMessageText(ToolGUICommon.MSG_CLEAR_PIN_CODE_COMMENT2);
            mainForm.OnPrintMessageText(ToolGUICommon.MSG_CLEAR_PIN_CODE_COMMENT3);

            // authenticatorResetコマンドを実行する
            byte[] commandByte = { 0x07 };
            hidProcess.SendHIDMessage(receivedCID, Const.HID_CMD_CTAPHID_CBOR, commandByte, commandByte.Length);
        }

        public void DoClientPinSet(string pinNew, string pinOld)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // 実行引数を退避
            clientPinNew = pinNew;
            clientPinOld = pinOld;
            // 実行するコマンドを退避
            requestType = HIDRequestType.ClientPinSet;
            cborCommand = Const.HID_CBORCMD_CLIENT_PIN;
            // nonce を送信する
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_CTAPHID_INIT, nonceBytes, nonceBytes.Length);
        }

        public void DoGetKeyAgreement(byte[] receivedCID)
        {
            // 実行するコマンドを退避
            cborCommand = Const.HID_CBORCMD_CLIENT_PIN;
            subCommand = Const.HID_SUBCMD_CLIENT_PIN_GET_AGREEMENT;
            // GetAgreementコマンドを実行する
            CBOREncoder cborEncoder = new CBOREncoder();
            byte[] getAgreementCbor = cborEncoder.GetKeyAgreement(cborCommand, subCommand);
            hidProcess.SendHIDMessage(receivedCID, Const.HID_CMD_CTAPHID_CBOR, getAgreementCbor, getAgreementCbor.Length);
        }

        private byte[] ExtractCBORBytesFromResponse(byte[] message, int length)
        {
            // レスポンスされたCBORを抽出
            //   CBORバイト配列はレスポンスの２バイト目以降
            int cborLength = length - 1;
            byte[] cborBytes = new byte[cborLength];
            for (int i = 0; i < cborLength; i++) {
                cborBytes[i] = message[1 + i];
            }
            return cborBytes;
        }

        private void DoResponseCtapHidCbor(byte[] message, int length)
        {
            // ステータスバイトをチェック
            if (CheckStatusByte(message) == false) {
                // 画面に制御を戻す
                mainForm.OnAppMainProcessExited(false);
                return;
            }

            switch (cborCommand) {
            case Const.HID_CBORCMD_CLIENT_PIN:
                DoResponseCommandClientPin(message, length);
                break;
            case Const.HID_CBORCMD_MAKE_CREDENTIAL:
                DoResponseCommandMakeCredential(message, length);
                break;
            case Const.HID_CBORCMD_GET_ASSERTION:
            case Const.HID_CBORCMD_AUTH_RESET:
                // 画面に制御を戻す
                mainForm.OnAppMainProcessExited(true);
                break;
            default:
                // 正しくレスポンスされなかったと判断し、画面に制御を戻す
                mainForm.OnAppMainProcessExited(false);
                break;
            }
        }

        private bool CheckStatusByte(byte[] message)
        {
            switch (message[0]) {
            case 0x00:
                return true;
            case 0x31:  // CTAP2_ERR_PIN_INVALID
            case 0x33:  // CTAP2_ERR_PIN_AUTH_INVALID:
                mainForm.OnPrintMessageText(AppCommon.MSG_CTAP2_ERR_PIN_INVALID);
                break;
            case 0x32:  // CTAP2_ERR_PIN_BLOCKED
                mainForm.OnPrintMessageText(AppCommon.MSG_CTAP2_ERR_PIN_BLOCKED);
                break;
            case 0x34:  // CTAP2_ERR_PIN_AUTH_BLOCKED
                mainForm.OnPrintMessageText(AppCommon.MSG_CTAP2_ERR_PIN_AUTH_BLOCKED);
                break;
            case 0x35:  // CTAP2_ERR_PIN_NOT_SET
                mainForm.OnPrintMessageText(AppCommon.MSG_CTAP2_ERR_PIN_NOT_SET);
                break;
            default:
                break;
            }
            return false;
        }

        private void DoResponseCommandClientPin(byte[] message, int length)
        {
            // レスポンスされたCBORを抽出
            byte[] cborBytes = ExtractCBORBytesFromResponse(message, length);

            switch (subCommand) {
            case Const.HID_SUBCMD_CLIENT_PIN_GET_AGREEMENT:
                DoResponseCommandGetKeyAgreement(cborBytes);
                break;
            case Const.HID_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN:
                DoResponseCommandGetPinToken(cborBytes);
                break;
            default:
                // 画面に制御を戻す
                mainForm.OnAppMainProcessExited(true);
                break;
            }
        }

        public void DoResponseCommandGetKeyAgreement(byte[] cborBytes)
        {
            switch (requestType) {
            case HIDRequestType.TestMakeCredential:
            case HIDRequestType.TestGetAssertion:
                // PINトークン取得処理を続行
                DoGetPinToken(cborBytes);
                break;
            default:
                // PIN設定処理を続行
                DoClientPinSetOrChange(cborBytes);
                break;
            }
        }

        public void DoResponseCommandGetPinToken(byte[] cborBytes)
        {
            switch (requestType) {
            case HIDRequestType.TestMakeCredential:
            case HIDRequestType.TestGetAssertion:
                // ログイン／認証処理を続行
                DoResponseGetPinToken(cborBytes);
                break;
            default:
                // 画面に制御を戻す
                mainForm.OnAppMainProcessExited(true);
                break;
            }
        }

        public void DoClientPinSetOrChange(byte[] cborBytes)
        {
            // 実行するコマンドを退避
            cborCommand = Const.HID_CBORCMD_CLIENT_PIN;
            byte[] receivedCID = hidProcess.receivedCID;

            if (clientPinOld.Equals(string.Empty)) {
                // SetPINコマンドを実行する
                subCommand = Const.HID_SUBCMD_CLIENT_PIN_SET;
            } else {
                // ChangePINコマンドを実行する
                subCommand = Const.HID_SUBCMD_CLIENT_PIN_CHANGE;
            }

            // リクエストデータ（CBOR）をエンコードして送信
            byte[] setPinCbor = new CBOREncoder().SetPIN(cborCommand, subCommand, clientPinNew, clientPinOld, cborBytes);
            hidProcess.SendHIDMessage(receivedCID, Const.HID_CMD_CTAPHID_CBOR, setPinCbor, setPinCbor.Length);
        }

        public void DoCtap2Healthcheck(string pin)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // 実行するコマンドと引数を退避
            //   認証器からPINトークンを取得するため、
            //   ClientPINコマンドを事前実行する必要あり
            requestType = HIDRequestType.TestMakeCredential;
            cborCommand = Const.HID_CBORCMD_CLIENT_PIN;
            clientPin = pin;
            // nonce を送信する
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_CTAPHID_INIT, nonceBytes, nonceBytes.Length);
        }

        public void DoGetPinToken(byte[] cborBytes)
        {
            // 実行するコマンドを退避
            cborCommand = Const.HID_CBORCMD_CLIENT_PIN;
            subCommand = Const.HID_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN;
            byte[] receivedCID = hidProcess.receivedCID;

            // リクエストデータ（CBOR）をエンコードして送信
            //   この時に生成した共通鍵は、モジュール内で保持しておく
            CBOREncoder encoder = new CBOREncoder();
            byte[] getPinTokenCbor = encoder.GetPinToken(cborCommand, subCommand, clientPin, cborBytes);
            SharedSecretKey = encoder.SharedSecretKey;
            hidProcess.SendHIDMessage(receivedCID, Const.HID_CMD_CTAPHID_CBOR, getPinTokenCbor, getPinTokenCbor.Length);
        }

        public void DoResponseGetPinToken(byte[] cborBytes)
        {
            // リクエストデータ（CBOR）をエンコード
            byte[] requestCbor = null;
            byte[] receivedCID = hidProcess.receivedCID;
            if (requestType == HIDRequestType.TestMakeCredential) {
                cborCommand = Const.HID_CBORCMD_MAKE_CREDENTIAL;
                requestCbor = new CBOREncoder().MakeCredential(cborCommand, clientPin, cborBytes, SharedSecretKey);
            } else {
                cborCommand = Const.HID_CBORCMD_GET_ASSERTION;
                requestCbor = new CBOREncoder().GetAssertion(cborCommand, clientPin, cborBytes, SharedSecretKey, MakeCredentialRes);
            }

            if (requestCbor == null) {
                // 処理失敗時は画面に制御を戻す
                mainForm.OnAppMainProcessExited(false);
                return;
            }

            // リクエスト転送の前に、
            // 基板上ののMAIN SWを押してもらうように促す
            // メッセージを画面表示
            if (requestType == HIDRequestType.TestGetAssertion) {
                mainForm.OnPrintMessageText("ログインテストを開始します.");
                mainForm.OnPrintMessageText("  ユーザー所在確認が必要となりますので、");
                mainForm.OnPrintMessageText("  FIDO認証器上のユーザー所在確認LEDが点滅したら、");
                mainForm.OnPrintMessageText("  MAIN SWを１回押してください.");
            }

            // リクエストを送信
            hidProcess.SendHIDMessage(receivedCID, Const.HID_CMD_CTAPHID_CBOR, requestCbor, requestCbor.Length);
        }

        private void DoResponseCommandMakeCredential(byte[] message, int length)
        {
            // レスポンスされたCBORを抽出
            byte[] cborBytes = ExtractCBORBytesFromResponse(message, length);
            // 次のGetAssertionリクエスト送信に必要となる
            // Credential IDを抽出して退避
            MakeCredentialRes = new CBORDecoder().MakeCredential(cborBytes);

            // 実行するコマンドと引数を退避
            //   認証器からPINトークンを取得するため、
            //   ClientPINコマンドを事前実行する必要あり
            requestType = HIDRequestType.TestGetAssertion;
            cborCommand = Const.HID_CBORCMD_CLIENT_PIN;
            // nonce を送信する
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_CTAPHID_INIT, nonceBytes, nonceBytes.Length);
        }
    }
}
