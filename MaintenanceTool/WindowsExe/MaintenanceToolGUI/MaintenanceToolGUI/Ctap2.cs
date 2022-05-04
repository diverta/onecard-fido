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
        private BLEMain bleMain;

        // メイン画面の参照を保持
        private MainForm mainForm;

        // PINGバイトを保持
        private byte[] pingBytes = new byte[100];

        // 実行中のサブコマンドを保持
        private byte cborCommand;
        private byte cborSubCommand;

        // 実行機能を保持
        private AppCommon.RequestType requestType;

        // ヘルスチェック処理の実行引数を退避
        private string clientPin;
        private string clientPinNew;

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

        public Ctap2(MainForm m, byte transportType_)
        {
            mainForm = m;
            transportType = transportType_;
        }

        public void SetHidMain(HIDMain p)
        {
            hidMain = p;
        }

        public void SetBleMain(BLEMain a)
        {
            bleMain = a;
        }

        public void SetClientPin(string p)
        {
            clientPin = p;
        }

        public void SetClientPinNew(string pn)
        {
            clientPinNew = pn;
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
                hidMain.SendHIDMessage(Const.HID_CMD_CTAPHID_PING, pingBytes, pingBytes.Length);
                break;
            case AppCommon.TRANSPORT_BLE:
                bleMain.SendBLEMessage(Const.HID_CMD_CTAPHID_PING, pingBytes, pingBytes.Length);
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
            case AppCommon.CTAP2_CBORCMD_GET_ASSERTION:
                DoResponseCommandGetAssertion(message, length);
                break;
            case AppCommon.CTAP2_CBORCMD_AUTH_RESET:
                DoResponseReset(message, length);
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
            byte[] cborBytes = AppCommon.ExtractCBORBytesFromResponse(message, length);

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

        //
        // ClientPINコマンド関連処理
        //
        public void DoGetKeyAgreement(AppCommon.RequestType t)
        {
            // 実行するコマンドを退避
            requestType = t;
            cborCommand = AppCommon.CTAP2_CBORCMD_CLIENT_PIN;
            cborSubCommand = AppCommon.CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT;

            // GetAgreementコマンドバイトを生成
            CBOREncoder cborEncoder = new CBOREncoder();
            byte[] getAgreementCbor = cborEncoder.GetKeyAgreement(cborCommand, cborSubCommand);

            // GetAgreementコマンドを実行
            switch (transportType) {
            case AppCommon.TRANSPORT_HID:
                hidMain.SendHIDMessage(Const.HID_CMD_CTAPHID_CBOR, getAgreementCbor, getAgreementCbor.Length);
                break;
            case AppCommon.TRANSPORT_BLE:
                bleMain.SendBLEMessage(U2f.Const.BLE_CMD_MSG, getAgreementCbor, getAgreementCbor.Length);
                break;
            default:
                break;
            }
        }

        public void DoResponseCommandGetKeyAgreement(byte[] cborBytes)
        {
            switch (requestType) {
            case AppCommon.RequestType.TestMakeCredential:
            case AppCommon.RequestType.TestGetAssertion:
                // PINトークン取得処理を続行
                DoGetPinToken(cborBytes);
                break;
            case AppCommon.RequestType.ClientPinSet:
                // PIN設定処理を続行
                DoClientPinSetOrChange(cborBytes);
                break;
            case AppCommon.RequestType.InstallSkeyCert:
                // 鍵・証明書インストール処理を続行
                hidMain.DoRequestInstallSkeyCert(cborBytes);
                break;
            default:
                // 画面に制御を戻す
                mainForm.OnAppMainProcessExited(false);
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
                hidMain.SendHIDMessage(Const.HID_CMD_CTAPHID_CBOR, getPinTokenCbor, getPinTokenCbor.Length);
                break;
            case AppCommon.TRANSPORT_BLE:
                bleMain.SendBLEMessage(U2f.Const.BLE_CMD_MSG, getPinTokenCbor, getPinTokenCbor.Length);
                break;
            default:
                break;
            }
        }

        public void DoResponseCommandGetPinToken(byte[] cborBytes)
        {
            switch (requestType) {
            case AppCommon.RequestType.TestMakeCredential:
            case AppCommon.RequestType.TestGetAssertion:
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
            if (requestType == AppCommon.RequestType.TestMakeCredential) {
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

            if (requestType == AppCommon.RequestType.TestGetAssertion && testUserPresenceNeeded) {
                // リクエスト転送の前に、
                // FIDO認証器のMAIN SWを押してもらうように促す
                // メッセージを画面表示
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_CTAP2_LOGIN_TEST_START);
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT1);
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT2);
                mainForm.OnPrintMessageText(AppCommon.MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT3);
            }

            // MakeCredential／GetAssertionコマンドを実行
            switch (transportType) {
            case AppCommon.TRANSPORT_HID:
                hidMain.SendHIDMessage(Const.HID_CMD_CTAPHID_CBOR, requestCbor, requestCbor.Length);
                break;
            case AppCommon.TRANSPORT_BLE:
                bleMain.SendBLEMessage(U2f.Const.BLE_CMD_MSG, requestCbor, requestCbor.Length);
                break;
            default:
                break;
            }
        }

        public void DoClientPinSetOrChange(byte[] cborBytes)
        {
            // 実行するコマンドを退避
            cborCommand = AppCommon.CTAP2_CBORCMD_CLIENT_PIN;

            if (clientPin.Equals(string.Empty)) {
                // 現在登録されているPINが指定されなかった場合
                // SetPINコマンドを実行する
                cborSubCommand = AppCommon.CTAP2_SUBCMD_CLIENT_PIN_SET;
            } else {
                // ChangePINコマンドを実行する
                cborSubCommand = AppCommon.CTAP2_SUBCMD_CLIENT_PIN_CHANGE;
            }

            // リクエストデータ（CBOR）をエンコードして送信
            byte[] setPinCbor = new CBOREncoder().SetPIN(cborCommand, cborSubCommand, clientPinNew, clientPin, cborBytes);
            hidMain.SendHIDMessage(Const.HID_CMD_CTAPHID_CBOR, setPinCbor, setPinCbor.Length);
        }

        //
        // MakeCredentialコマンド関連処理
        //
        private void DoResponseCommandMakeCredential(byte[] message, int length)
        {
            // レスポンスされたCBORを抽出
            byte[] cborBytes = AppCommon.ExtractCBORBytesFromResponse(message, length);
            // 次のGetAssertionリクエスト送信に必要となる
            // Credential IDを抽出して退避
            MakeCredentialRes = new CBORDecoder().CreateOrGetCommand(cborBytes, true);

            // GetAssertionコマンドを実行する
            GetAssertionCount = 1;
            DoRequestCommandGetAssertion();
        }

        //
        // GetAssertionコマンド関連処理
        //
        private void DoRequestCommandGetAssertion()
        {
            // 実行するコマンドと引数を退避
            //   認証器からPINトークンを取得するため、
            //   ClientPINコマンドを事前実行する必要あり
            requestType = AppCommon.RequestType.TestGetAssertion;
            cborCommand = AppCommon.CTAP2_CBORCMD_CLIENT_PIN;

            switch (transportType) {
            case AppCommon.TRANSPORT_HID:
                // INITコマンドを実行し、nonce を送信する
                hidMain.DoRequestCtapHidInit(requestType);
                break;
            case AppCommon.TRANSPORT_BLE:
                // 再度、GetKeyAgreementコマンドを実行
                DoGetKeyAgreement(requestType);
                break;
            default:
                break;
            }
        }

        private void DoResponseCommandGetAssertion(byte[] message, int length)
        {
            // GetAssertion実行が２回目かどうか判定
            bool verifySaltNeeded = (GetAssertionCount == 2);

            // レスポンスされたCBORを抽出
            byte[] cborBytes = AppCommon.ExtractCBORBytesFromResponse(message, length);
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
                AppCommon.OutputLogDebug(string.Format(
                    "authenticatorGetAssertion: hmac-secret-salt verify {0}", success ? "success" : "failed")
                    );
                return success;

            } else {
                // １回目のGetAssertionの場合はオリジナルSaltを抽出して終了
                DecryptedSaltOrg = AppCommon.AES256CBCDecrypt(SharedSecretKey, encryptedSalt);
                return true;
            }
        }

        //
        // Resetコマンド関連処理
        //
        public void DoRequestAuthReset()
        {
            // コマンドを退避
            cborCommand = AppCommon.CTAP2_CBORCMD_AUTH_RESET;

            // リクエスト転送の前に、
            // 基板上ののMAIN SWを押してもらうように促す
            // メッセージを画面表示
            mainForm.OnPrintMessageText(ToolGUICommon.MSG_CLEAR_PIN_CODE_COMMENT1);
            mainForm.OnPrintMessageText(ToolGUICommon.MSG_CLEAR_PIN_CODE_COMMENT2);
            mainForm.OnPrintMessageText(ToolGUICommon.MSG_CLEAR_PIN_CODE_COMMENT3);

            // authenticatorResetコマンドを実行する
            byte[] commandByte = { AppCommon.CTAP2_CBORCMD_AUTH_RESET };
            hidMain.SendHIDMessage(Const.HID_CMD_CTAPHID_CBOR, commandByte, commandByte.Length);
        }

        private void DoResponseReset(byte[] message, int length)
        {
            // 画面に制御を戻す
            mainForm.OnAppMainProcessExited(true);
        }
    }
}
