using MaintenanceToolCommon;
using System;

namespace MaintenanceToolGUI
{
    internal static class Const
    {
        // HIDフレームに関する定義
        public const int HID_FRAME_LEN = 64;
        public const int HID_INIT_HEADER_LEN = 7;
        public const int HID_CONT_HEADER_LEN = 5;
        // HIDコマンドバイトに関する定義
        public const int HID_CMD_CTAPHID_PING = 0x81;
        public const int HID_CMD_CTAPHID_INIT = 0x86;
        public const int HID_CMD_ERASE_SKEY_CERT = 0xc0;
        public const int HID_CMD_INSTALL_SKEY_CERT = 0xc1;
        public const int HID_CMD_GET_FLASH_STAT = 0xc2;
        public const int HID_CMD_CTAPHID_CBOR = 0x90;
        public const int HID_CMD_UNKNOWN_ERROR = 0xbf;
        // BLEコマンドバイトに関する定義
        public const int BLE_CMD_MSG = 0x83;
    }

    internal class HIDMain
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
        private KeyAgreement AgreementPublicKey = null;

        // ユーザー登録情報を退避
        //   makeCredential時に受信したユーザー登録情報を、
        //   getAssertion実行時まで保持しておく
        private CreateOrGetCommandResponse MakeCredentialRes = null;

        // GetAssertion実行回数を保持
        private int GetAssertionCount;

        // １回目のGetAssertion実行時、
        // hmac-secret拡張から抽出／復号化されたsaltを保持
        private byte[] DecryptedSaltOrg;

        // 実行機能を保持
        private enum HIDRequestType
        {
            None = 0,
            ClientPinSet,
            TestCtapHidPing,
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

            // CTAP2共通処理に各種参照を引き渡す
            ctap2 = new Ctap2(mainForm, AppCommon.TRANSPORT_HID);
            ctap2.setHidMain(this);
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
            case Const.HID_CMD_CTAPHID_PING:
                ctap2.DoResponsePing(message, length);
                break;
            case Const.HID_CMD_CTAPHID_INIT:
                DoResponseTestCtapHidInit(message, length);
                break;
            case Const.HID_CMD_ERASE_SKEY_CERT:
            case Const.HID_CMD_INSTALL_SKEY_CERT:
                DoResponseMaintSkeyCert(message, length);
                break;
            case Const.HID_CMD_GET_FLASH_STAT:
                DoResponseGetFlashStat(message, length);
                break;
            case Const.HID_CMD_CTAPHID_CBOR:
                DoResponseCtapHidCbor(message, length);
                break;
            case Const.HID_CMD_UNKNOWN_ERROR:
                // 画面に制御を戻す
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

        public void DoTestCtapHidPing()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // 実行するコマンドを退避
            requestType = HIDRequestType.TestCtapHidPing;
            cborCommand = AppCommon.CTAP2_CBORCMD_NONE;
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit();
        }

        public void SendHIDMessage(byte cmd, byte[] message)
        {
            if (ReceivedCID != null) {
                hidProcess.SendHIDMessage(ReceivedCID, cmd, message, message.Length);
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

        private void DoRequestCtapHidInit()
        {
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

            if (cborCommand == AppCommon.CTAP2_CBORCMD_CLIENT_PIN) {
                // レスポンスされたCIDを抽出し、PIN設定処理を続行
                DoGetKeyAgreement(ReceivedCID);
            } else if (cborCommand == AppCommon.CTAP2_CBORCMD_AUTH_RESET) {
                // レスポンスされたCIDを抽出し、Reset処理を続行
                DoRequestAuthReset(ReceivedCID);
            } else {
                // PINGコマンドを実行
                ctap2.DoRequestPing();
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

        public void DoGetFlashStat()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // コマンドバイトだけを送信する
            hidProcess.SendHIDMessage(CIDBytes, Const.HID_CMD_GET_FLASH_STAT, RequestData, 0);
        }

        private void DoResponseGetFlashStat(byte[] message, int length)
        {
            // 戻りメッセージから、取得情報CSVを抽出
            byte[] responseBytes = ExtractCBORBytesFromResponse(message, length);
            string responseCSV = System.Text.Encoding.ASCII.GetString(responseBytes);
            AppCommon.OutputLogToFile("Flash ROM statistics: " + responseCSV, true);

            // 情報取得CSVから空き領域に関する情報を抽出
            string[] vars = responseCSV.Split(',');
            string strRemain = "";
            string strAvail = "";
            string strCorrupt = "";
            foreach (string v in vars) {
                if (v.StartsWith("largest_contig=")) {
                    strRemain = v.Split('=')[1];
                }
                else if (v.StartsWith("words_available=")) {
                    strAvail = v.Split('=')[1];
                }
                else if (v.StartsWith("corruption=")) {
                    strCorrupt = v.Split('=')[1];
                }
            }

            // 空き容量、破損状況を画面に表示
            string rateText = "";
            if (strRemain.Length > 0 && strAvail.Length > 0) {
                float rate = float.Parse(strRemain) / float.Parse(strAvail) * 100;
                rateText = string.Format(AppCommon.MSG_FSTAT_REMAINING_RATE, rate);
            } else {
                rateText = AppCommon.MSG_FSTAT_NON_REMAINING_RATE;
            }
            string corruptText = (strCorrupt.Equals("0")) ? 
                AppCommon.MSG_FSTAT_CORRUPTING_AREA_NOT_EXIST : AppCommon.MSG_FSTAT_CORRUPTING_AREA_EXIST;

            // 画面に制御を戻す
            mainForm.OnPrintMessageText(string.Format("  {0}{1}", rateText, corruptText));
            mainForm.OnAppMainProcessExited(true);
        }

        public void DoAuthReset()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // 実行するコマンドを退避
            requestType = HIDRequestType.AuthReset;
            cborCommand = AppCommon.CTAP2_CBORCMD_AUTH_RESET;
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit();
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
            cborCommand = AppCommon.CTAP2_CBORCMD_CLIENT_PIN;
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit();
        }

        public void DoGetKeyAgreement(byte[] receivedCID)
        {
            // 実行するコマンドを退避
            cborCommand = AppCommon.CTAP2_CBORCMD_CLIENT_PIN;
            subCommand = AppCommon.CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT;
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
            case AppCommon.CTAP2_CBORCMD_CLIENT_PIN:
                DoResponseCommandClientPin(message, length);
                break;
            case AppCommon.CTAP2_CBORCMD_MAKE_CREDENTIAL:
                DoResponseCommandMakeCredential(message, length);
                break;
            case AppCommon.CTAP2_CBORCMD_GET_ASSERTION:
                DoResponseCommandGetAssertion(message, length);
                break;
            case AppCommon.CTAP2_CBORCMD_AUTH_RESET:
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
            case 0xfe:  // CTAP2_ERR_VENDOR_KEY_CRT_NOT_EXIST (CTAP2_ERR_VENDOR_FIRST+0x0e)
                mainForm.OnPrintMessageText(AppCommon.MSG_OCCUR_SKEYNOEXIST_ERROR);
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
            case AppCommon.CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT:
                DoResponseCommandGetKeyAgreement(cborBytes);
                break;
            case AppCommon.CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN:
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
            cborCommand = AppCommon.CTAP2_CBORCMD_CLIENT_PIN;
            byte[] receivedCID = hidProcess.receivedCID;

            if (clientPinOld.Equals(string.Empty)) {
                // SetPINコマンドを実行する
                subCommand = AppCommon.CTAP2_SUBCMD_CLIENT_PIN_SET;
            } else {
                // ChangePINコマンドを実行する
                subCommand = AppCommon.CTAP2_SUBCMD_CLIENT_PIN_CHANGE;
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
            cborCommand = AppCommon.CTAP2_CBORCMD_CLIENT_PIN;
            clientPin = pin;
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit();
        }

        public void DoGetPinToken(byte[] cborBytes)
        {
            // 実行するコマンドを退避
            cborCommand = AppCommon.CTAP2_CBORCMD_CLIENT_PIN;
            subCommand = AppCommon.CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN;
            byte[] receivedCID = hidProcess.receivedCID;

            // リクエストデータ（CBOR）をエンコードして送信
            //   この時に生成した共通鍵は、モジュール内で保持しておく
            CBOREncoder encoder = new CBOREncoder();
            byte[] getPinTokenCbor = encoder.GetPinToken(cborCommand, subCommand, clientPin, cborBytes);
            SharedSecretKey = encoder.SharedSecretKey;
            AgreementPublicKey = encoder.AgreementPublicKey;
            hidProcess.SendHIDMessage(receivedCID, Const.HID_CMD_CTAPHID_CBOR, getPinTokenCbor, getPinTokenCbor.Length);
        }

        public void DoResponseGetPinToken(byte[] cborBytes)
        {
            // GetAssertion実行が２回目かどうか判定
            bool testUserPresenceNeeded = (GetAssertionCount == 2);

            // リクエストデータ（CBOR）をエンコード
            byte[] requestCbor = null;
            byte[] receivedCID = hidProcess.receivedCID;
            if (requestType == HIDRequestType.TestMakeCredential) {
                cborCommand = AppCommon.CTAP2_CBORCMD_MAKE_CREDENTIAL;
                requestCbor = new CBOREncoder().MakeCredential(cborCommand, clientPin, cborBytes, SharedSecretKey);
            } else {
                // ２回目のGetAssertion実行では、MAIN SW押下によるユーザー所在確認が必要
                cborCommand = AppCommon.CTAP2_CBORCMD_GET_ASSERTION;
                requestCbor = new CBOREncoder().GetAssertion(cborCommand, clientPin, cborBytes, SharedSecretKey, MakeCredentialRes, AgreementPublicKey, testUserPresenceNeeded);
            }

            if (requestCbor == null) {
                // 処理失敗時は画面に制御を戻す
                mainForm.OnAppMainProcessExited(false);
                return;
            }

            // リクエスト転送の前に、
            // 基板上のMAIN SWを押してもらうように促す
            // メッセージを画面表示
            if (requestType == HIDRequestType.TestGetAssertion && testUserPresenceNeeded) {
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
            MakeCredentialRes = new CBORDecoder().CreateOrGetCommand(cborBytes, true);

            // GetAssertionコマンドを実行する
            GetAssertionCount = 1;
            DoRequestCommandGetAssertion();
        }

        private void DoResponseCommandGetAssertion(byte[] message, int length)
        {
            // GetAssertion実行が２回目かどうか判定
            bool verifySaltNeeded = (GetAssertionCount == 2);
 
            // レスポンスされたCBORを抽出
            byte[] cborBytes = ExtractCBORBytesFromResponse(message, length);
            // hmac-secret拡張情報からsaltを抽出して保持
            CreateOrGetCommandResponse resp = new CBORDecoder().CreateOrGetCommand(cborBytes, false);
            if (VerifyHmacSecretSalt(resp.HmacSecretRes.Output, verifySaltNeeded) == false) {
                // salt検証失敗時は画面に制御を戻す
                mainForm.OnAppMainProcessExited(false);
                return;
            }

            if (verifySaltNeeded) {
                // ２回目のテストが成功したら画面に制御を戻して終了
                mainForm.OnAppMainProcessExited(true);
                return;
            }

            // GetAssertionコマンドを実行する
            GetAssertionCount++;
            DoRequestCommandGetAssertion();
        }

        private void DoRequestCommandGetAssertion()
        {
            // 実行するコマンドと引数を退避
            //   認証器からPINトークンを取得するため、
            //   ClientPINコマンドを事前実行する必要あり
            requestType = HIDRequestType.TestGetAssertion;
            cborCommand = AppCommon.CTAP2_CBORCMD_CLIENT_PIN;
            // INITコマンドを実行し、nonce を送信する
            DoRequestCtapHidInit();
        }

        private bool VerifyHmacSecretSalt(byte[] encryptedSalt, bool verifySaltNeeded)
        {
            // レスポンス内に"hmac-secret"拡張が含まれていない場合はここで終了
            if (encryptedSalt == null) {
                return true;
            }

            if (verifySaltNeeded) {
                // １回目のGetAssertionの場合はオリジナルSaltと内容を比較し、
                // 同じ内容であれば検証成功
                byte[] decryptedSaltCur = AppCommon.AES256CBCDecrypt(SharedSecretKey, encryptedSalt);
                bool success = AppCommon.CompareBytes(decryptedSaltCur, DecryptedSaltOrg, ExtHmacSecretResponse.OutputSize);

                // 検証結果はログファイル出力する
                AppCommon.OutputLogToFile(string.Format(
                    "authenticatorGetAssertion: hmac-secret-salt verify {0}", success ? "success" : "failed"),
                    true);
                return success;

            } else {
                // １回目のGetAssertionの場合はオリジナルSaltを抽出して終了
                DecryptedSaltOrg = AppCommon.AES256CBCDecrypt(SharedSecretKey, encryptedSalt);
                return true;
            }
        }
    }
}
