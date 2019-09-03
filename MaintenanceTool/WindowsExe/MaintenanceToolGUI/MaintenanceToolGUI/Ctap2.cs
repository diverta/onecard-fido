using MaintenanceToolCommon;
using System;

namespace MaintenanceToolGUI
{
    class Ctap2
    {
        // トランスポート種別を保持
        private byte transportType;

        // トランスポート別処理の参照を保持
        private HIDMain hidMain;
        private AppMain bleMain;

        // メイン画面の参照を保持
        private MainForm mainForm;

        // PINGバイトを保持
        private byte[] pingBytes = new byte[100];

        // 実行中のサブコマンドを保持
        private byte cborCommand;
        private byte cborSubCommand;

        // 実行機能を保持
        public enum RequestType
        {
            None = 0,
            ClientPinSet,
            TestCtapHidPing,
            TestMakeCredential,
            TestGetAssertion,
            AuthReset
        };
        private RequestType requestType;

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

        public Ctap2(MainForm m, byte transportType_)
        {
            mainForm = m;
            transportType = transportType_;
        }

        public void setHidMain(HIDMain p)
        {
            hidMain = p;
        }

        public void setBleMain(AppMain a)
        {
            bleMain = a;
        }

        public void setRequestType(RequestType t)
        {
            requestType = t;
        }

        public void setClientPin(string p)
        {
            clientPin = p;
        }

        //
        // PINGコマンド関連処理
        //
        public void DoRequestPing()
        {
            // コマンドを退避
            cborCommand = AppCommon.CTAP2_CBORCMD_NONE;

            // 100バイトのランダムデータを生成
            new Random().NextBytes(pingBytes);

            // PINGコマンドを実行する
            switch (transportType) {
            case AppCommon.TRANSPORT_HID:
                hidMain.SendHIDMessage(Const.HID_CMD_CTAPHID_PING, pingBytes);
                break;
            case AppCommon.TRANSPORT_BLE:
                bleMain.SendBLEMessage(Const.HID_CMD_CTAPHID_PING, pingBytes);
                break;
            default:
                break;
            }
        }

        public void DoResponsePing(byte[] message, int length)
        {
            // PINGバイトの一致チェック
            bool result = true;
            for (int i = 0; i < pingBytes.Length; i++) {
                if (pingBytes[i] != message[i]) {
                    // 画面のテキストエリアにメッセージを表示
                    mainForm.OnPrintMessageText(AppCommon.MSG_CMDTST_INVALID_PING);
                    result = false;
                    break;
                }
            }
            // 画面に制御を戻す
            mainForm.OnAppMainProcessExited(result);
        }

        //
        // CBORコマンド関連処理
        //
        public void DoResponseCtapHidCbor(byte[] message, int length)
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
            /* 
             * TODO:後日追加
             * 
            case AppCommon.CTAP2_CBORCMD_GET_ASSERTION:
                DoResponseCommandGetAssertion(message, length);
                break;
            */
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

            switch (cborSubCommand) {
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

        //
        // ClientPINコマンド関連処理
        //
        public void DoGetKeyAgreement()
        {
            // 実行するコマンドを退避
            cborCommand = AppCommon.CTAP2_CBORCMD_CLIENT_PIN;
            cborSubCommand = AppCommon.CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT;

            // GetAgreementコマンドバイトを生成
            CBOREncoder cborEncoder = new CBOREncoder();
            byte[] getAgreementCbor = cborEncoder.GetKeyAgreement(cborCommand, cborSubCommand);

            // GetAgreementコマンドを実行
            switch (transportType) {
            case AppCommon.TRANSPORT_HID:
                hidMain.SendHIDMessage(Const.HID_CMD_CTAPHID_CBOR, getAgreementCbor);
                break;
            case AppCommon.TRANSPORT_BLE:
                bleMain.SendBLEMessage(Const.BLE_CMD_MSG, getAgreementCbor);
                break;
            default:
                break;
            }
        }

        public void DoResponseCommandGetKeyAgreement(byte[] cborBytes)
        {
            switch (requestType) {
            case RequestType.TestMakeCredential:
            case RequestType.TestGetAssertion:
                // PINトークン取得処理を続行
                DoGetPinToken(cborBytes);
                break;
            default:
                // TODO: 後日移行
                // PIN設定処理を続行
                // DoClientPinSetOrChange(cborBytes);
                break;
            }
        }

        public void DoGetPinToken(byte[] cborBytes)
        {
            // 実行するコマンドを退避
            cborCommand = AppCommon.CTAP2_CBORCMD_CLIENT_PIN;
            cborSubCommand = AppCommon.CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN;

            // リクエストデータ（CBOR）をエンコードして送信
            //   この時に生成した共通鍵は、モジュール内で保持しておく
            CBOREncoder encoder = new CBOREncoder();
            byte[] getPinTokenCbor = encoder.GetPinToken(cborCommand, cborSubCommand, clientPin, cborBytes);
            SharedSecretKey = encoder.SharedSecretKey;
            AgreementPublicKey = encoder.AgreementPublicKey;

            // GetPinTokenコマンドを実行
            switch (transportType) {
            case AppCommon.TRANSPORT_HID:
                hidMain.SendHIDMessage(Const.HID_CMD_CTAPHID_CBOR, getPinTokenCbor);
                break;
            case AppCommon.TRANSPORT_BLE:
                bleMain.SendBLEMessage(Const.BLE_CMD_MSG, getPinTokenCbor);
                break;
            default:
                break;
            }
        }

        public void DoResponseCommandGetPinToken(byte[] cborBytes)
        {
            switch (requestType) {
            case RequestType.TestMakeCredential:
            case RequestType.TestGetAssertion:
                // ログイン／認証処理を続行
                DoResponseGetPinToken(cborBytes);
                break;
            default:
                // 画面に制御を戻す
                mainForm.OnAppMainProcessExited(true);
                break;
            }
        }

        public void DoResponseGetPinToken(byte[] cborBytes)
        {
            // GetAssertion実行が２回目かどうか判定
            bool testUserPresenceNeeded = (GetAssertionCount == 2);

            // リクエストデータ（CBOR）をエンコード
            byte[] requestCbor = null;
            if (requestType == RequestType.TestMakeCredential) {
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
            if (requestType == RequestType.TestGetAssertion && testUserPresenceNeeded) {
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_CTAP2_LOGIN_TEST_START);
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT1);
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT2);
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT3);
            }

            // MakeCredential／GetAssertionコマンドを実行
            switch (transportType) {
            case AppCommon.TRANSPORT_HID:
                hidMain.SendHIDMessage(Const.HID_CMD_CTAPHID_CBOR, requestCbor);
                break;
            case AppCommon.TRANSPORT_BLE:
                bleMain.SendBLEMessage(Const.BLE_CMD_MSG, requestCbor);
                break;
            default:
                break;
            }
        }

        //
        // MakeCredentialコマンド関連処理
        //
        private void DoResponseCommandMakeCredential(byte[] message, int length)
        {
            // レスポンスされたCBORを抽出
            byte[] cborBytes = ExtractCBORBytesFromResponse(message, length);
            // 次のGetAssertionリクエスト送信に必要となる
            // Credential IDを抽出して退避
            MakeCredentialRes = new CBORDecoder().CreateOrGetCommand(cborBytes, true);

            // GetAssertionコマンドを実行する
            GetAssertionCount = 1;
            //DoRequestCommandGetAssertion();
            mainForm.OnAppMainProcessExited(true); // 仮コードです
        }
    }
}
