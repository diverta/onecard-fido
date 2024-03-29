﻿using MaintenanceToolApp.Common;
using System;
using System.Text;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;
using static MaintenanceToolApp.FIDODefine;

namespace MaintenanceToolApp.HealthCheck
{
    internal class CTAP2HealthCheckParameter
    {
        // 実行中のサブコマンドを保持
        public byte CborCommand { get; set; }
        public byte CborSubCommand { get; set; }

        // 公開鍵を保持
        public KeyAgreement AgreementPublicKey { get; set; }

        // 共通鍵を退避
        //   getPinToken時に生成した共通鍵を、
        //   makeCredential、getAssertion実行時まで保持しておく
        public byte[] SharedSecretKey { get; set; }

        // pinHashEnc: 
        //   Encrypted first 16 bytes of SHA-256 hash of curPin 
        //   using sharedSecret
        //   AES256-CBC(sharedSecret, IV= 0, LEFT(SHA-256(curPin),16))
        public byte[] PinHashEnc { get; set; }

        // ヘルスチェック実行用テストデータを保持
        public CTAP2HealthCheckTestData TestData { get; set; }

        // MakeCredentialレスポンスデータを保持
        public MakeOrGetCommandResponse MakeCredentialCommandResponse { get; set; }

        // GetAssertion実行回数を保持
        public int GetAssertionCount { get; set; }

        // １回目のGetAssertion実行時、
        // hmac-secret拡張から抽出／復号化されたsaltを保持
        public byte[] DecryptedSaltOrg { get; set; }

        public CTAP2HealthCheckParameter()
        {
            AgreementPublicKey = new KeyAgreement();
            SharedSecretKey = new byte[0];
            PinHashEnc = new byte[0];
            TestData = null!;
            MakeCredentialCommandResponse = null!;
            GetAssertionCount = 0;
            DecryptedSaltOrg = new byte[0];
        }
    }

    internal class CTAP2HealthCheckProcess
    {
        // デバッグログ出力
        private static readonly bool OutputDebugLog = false;

        // 処理実行のためのプロパティー
        private readonly HealthCheckParameter Parameter;

        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyCommandTerminated(string commandTitle, string errorMessage, bool success);
        private event HandlerOnNotifyCommandTerminated NotifyCommandTerminated = null!;

        // HID／BLEからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        public CTAP2HealthCheckProcess(HealthCheckParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;

            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
        }

        //
        // 外部公開用
        //
        public void DoRequestBleCtap2HealthCheck(HandlerOnNotifyCommandTerminated handler)
        {
            // 戻り先の関数を保持
            NotifyCommandTerminated = handler;

            // 実行コマンドを設定
            Parameter.Command = Command.COMMAND_TEST_MAKE_CREDENTIAL;

            // getKeyAgreementサブコマンドから実行
            DoRequestCommandGetKeyAgreement();
        }

        public void DoRequestHidCtap2HealthCheck(HandlerOnNotifyCommandTerminated handler)
        {
            // 戻り先の関数を保持
            NotifyCommandTerminated = handler;

            // 実行コマンドを設定
            Parameter.Command = Command.COMMAND_TEST_MAKE_CREDENTIAL;

            // CTAPHID_INITから実行
            DoRequestCtapHidInit();
        }

        //
        // 内部処理
        //
        // CTAP2ヘルスチェック処理で必要なパラメーターを保持
        private readonly CTAP2HealthCheckParameter HCheckParameter = new CTAP2HealthCheckParameter();

        //
        // INITコマンド関連処理
        //
        private void DoRequestCtapHidInit()
        {
            // INITコマンドを実行し、nonce を送信する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCtapHidInit();
        }

        private void DoResponseCtapHidInit()
        {
            // CTAPHID_INIT応答後の処理を実行
            switch (Parameter.Command) {
            case Command.COMMAND_TEST_MAKE_CREDENTIAL:
            case Command.COMMAND_TEST_GET_ASSERTION:
                DoRequestCommandGetKeyAgreement();
                break;
            default:
                // メイン画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
                break;
            }
        }

        //
        // ClientPINコマンド関連処理
        //
        private void DoRequestCommandGetKeyAgreement()
        {
            // 実行するコマンドを保持
            HCheckParameter.CborCommand = CTAP2_CBORCMD_CLIENT_PIN;
            HCheckParameter.CborSubCommand = CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT;

            // GetAgreementコマンドバイトを生成
            CBOREncoder cborEncoder = new CBOREncoder();
            byte[] getAgreementCbor = CBOREncoder.GenerateGetKeyAgreementCbor(HCheckParameter.CborCommand, HCheckParameter.CborSubCommand);

            // GetAgreementコマンドを実行
            switch (Parameter.Transport) {
            case Transport.TRANSPORT_HID:
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestCtapHidCommand(HIDProcessConst.HID_CMD_CTAPHID_CBOR, getAgreementCbor);
                break;
            case Transport.TRANSPORT_BLE:
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestBleCommand(0x80 | FIDO_CMD_MSG, getAgreementCbor);
                break;
            default:
                break;
            }
        }

        private void DoResponseCommandGetKeyAgreement(byte[] cborBytes)
        {
            // CBORをデコードして公開鍵を抽出
            CBORDecoder cborDecoder = new CBORDecoder();
            HCheckParameter.AgreementPublicKey = CBORDecoder.GetKeyAgreement(cborBytes);

            switch (Parameter.Command) {
            case Command.COMMAND_TEST_MAKE_CREDENTIAL:
            case Command.COMMAND_TEST_GET_ASSERTION:
                // PINトークン取得処理を続行
                DoRequestCommandGetPinToken();
                break;
            default:
                // 正しくレスポンスされなかったと判断し、画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
                break;
            }
        }

        private void DoRequestCommandGetPinToken()
        {
            // 実行するコマンドを退避
            HCheckParameter.CborCommand = CTAP2_CBORCMD_CLIENT_PIN;
            HCheckParameter.CborSubCommand = CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN;

            // 共通鍵を生成
            if (CTAP2Util.GenerateSharedSecretKey(HCheckParameter) == false) {
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_CTAP2_ERR_PIN_AUTH_SSKEY_GENERATE, false);
                return;
            }

            // PinHashEncを生成
            CTAP2Util.GeneratePinHashEnc(Parameter.Pin, HCheckParameter);

            // GetPinTokenコマンドバイトを生成
            CBOREncoder cborEncoder = new CBOREncoder();
            byte[] getPinTokenCbor = CBOREncoder.GenerateGetPinTokenCbor(
                HCheckParameter.CborCommand, HCheckParameter.CborSubCommand,
                HCheckParameter.AgreementPublicKey, HCheckParameter.PinHashEnc);

            // GetPinTokenコマンドを実行
            switch (Parameter.Transport) {
            case Transport.TRANSPORT_HID:
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestCtapHidCommand(HIDProcessConst.HID_CMD_CTAPHID_CBOR, getPinTokenCbor);
                break;
            case Transport.TRANSPORT_BLE:
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestBleCommand(0x80 | FIDO_CMD_MSG, getPinTokenCbor);
                break;
            default:
                break;
            }
        }

        private void DoResponseCommandGetPinToken(byte[] cborBytes)
        {
            // PinTokenをCBORから抽出
            byte[] pinTokenEnc = CBORDecoder.GetPinTokenEnc(cborBytes);
            if (pinTokenEnc.Length == 0) {
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_CTAP2_ERR_PIN_AUTH_TOKEN_GET, false);
                return;
            }

            // PinTokenを共通鍵で復号化
            byte[] pinToken = CTAP2Util.AES256CBCDecrypt(HCheckParameter.SharedSecretKey, pinTokenEnc);

            if (OutputDebugLog) {
                AppLogUtil.OutputLogDebug(string.Format(
                    "pinToken: \n{0}", AppLogUtil.DumpMessage(pinToken, pinToken.Length)));
            }

            switch (Parameter.Command) {
            case Command.COMMAND_TEST_MAKE_CREDENTIAL:
                // MakeCredentialコマンドを実行
                DoRequestCommandMakeCredential(pinToken);
                break;
            case Command.COMMAND_TEST_GET_ASSERTION:
                // GetAssertionコマンドを実行
                DoRequestCommandGetAssertion(pinToken);
                break;
            default:
                // 正しくレスポンスされなかったと判断し、画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
                break;
            }
        }

        //
        // MakeCredentialコマンド関連
        //
        private void DoRequestCommandMakeCredential(byte[] pinToken)
        {
            // 実行するコマンドを設定
            HCheckParameter.CborCommand = CTAP2_CBORCMD_MAKE_CREDENTIAL;

            // テストデータを生成
            HCheckParameter.TestData = new CTAP2HealthCheckTestData();

            // clientDataHashを生成
            byte[] clientDataHash = CTAP2Util.ComputeClientDataHash(HCheckParameter.TestData.Challenge);

            // pinAuthを生成
            byte[] pinAuth = CTAP2Util.GenerateClientPinAuth(pinToken, clientDataHash);

            // MakeCredentialコマンドバイトを生成
            byte[] makeCredentialCbor = CBOREncoder.GenerateMakeCredentialCbor(
                HCheckParameter.CborCommand, HCheckParameter.TestData.MakeCredentialParameter, clientDataHash, pinAuth);

            // MakeCredentialコマンドを実行
            switch (Parameter.Transport) {
            case Transport.TRANSPORT_HID:
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestCtapHidCommand(HIDProcessConst.HID_CMD_CTAPHID_CBOR, makeCredentialCbor);
                break;
            case Transport.TRANSPORT_BLE:
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestBleCommand(0x80 | FIDO_CMD_MSG, makeCredentialCbor);
                break;
            default:
                break;
            }
        }

        private void DoResponseCommandMakeCredential(byte[] message)
        {
            // レスポンスされたCBORを抽出
            byte[] cborBytes = AppUtil.ExtractCBORBytesFromResponse(message, message.Length);

            // MakeCredentialレスポンスデータを保持
            //   次のGetAssertionリクエスト送信に必要となる
            //   Credential IDを抽出して退避
            HCheckParameter.MakeCredentialCommandResponse = CBORDecoder.ParseMakeOrGetCommandResponse(cborBytes, true);

            // GetAssertionコマンドを実行する
            HCheckParameter.GetAssertionCount = 1;
            DoPrepareCommandGetAssertion();
        }

        //
        // GetAssertionコマンド関連
        //
        private void DoPrepareCommandGetAssertion()
        {
            // 実行するコマンドを設定
            //   認証器からPINトークンを取得するため、
            //   ClientPINコマンドを事前実行する必要あり
            Parameter.Command = Command.COMMAND_TEST_GET_ASSERTION;

            switch (Parameter.Transport) {
            case Transport.TRANSPORT_HID:
                // CTAPHID_INITから実行
                DoRequestCtapHidInit();
                break;
            case Transport.TRANSPORT_BLE:
                // 再度、GetKeyAgreementコマンドを実行
                DoRequestCommandGetKeyAgreement();
                break;
            default:
                break;
            }
        }

        private void DoRequestCommandGetAssertion(byte[] pinToken)
        {
            // GetAssertion実行が２回目かどうか判定
            //   ２回目のGetAssertion実行では、MAIN SW押下によるユーザー所在確認が必要
            bool testUserPresenceNeeded = (HCheckParameter.GetAssertionCount == 2);

            // 実行するコマンドを設定
            HCheckParameter.CborCommand = CTAP2_CBORCMD_GET_ASSERTION;

            // clientDataHashを生成
            byte[] clientDataHash = CTAP2Util.ComputeClientDataHash(HCheckParameter.TestData.Challenge);

            // pinAuthを生成
            byte[] pinAuth = CTAP2Util.GenerateClientPinAuth(pinToken, clientDataHash);

            // saltEncを生成
            byte[] saltEnc = CTAP2Util.GenerateSaltEnc(HCheckParameter.SharedSecretKey, HCheckParameter.TestData.Salt);

            // saltAuthを生成
            byte[] saltAuth = CTAP2Util.GenerateSaltAuth(HCheckParameter.SharedSecretKey, saltEnc);

            // Credential idを抽出
            byte[] credentialID = HCheckParameter.MakeCredentialCommandResponse.CredentialId;

            // GetAssertionコマンドバイトを生成
            string rpid = HCheckParameter.TestData.MakeCredentialParameter.RpId;
            byte[] getAssertionCbor = CBOREncoder.GenerateGetAssertionCbor(
                HCheckParameter.CborCommand, rpid, clientDataHash, credentialID, pinAuth,
                HCheckParameter.AgreementPublicKey, saltEnc, saltAuth, testUserPresenceNeeded);

            if (testUserPresenceNeeded) {
                // リクエスト転送の前に、
                // FIDO認証器のMAIN SWを押してもらうように促す
                // メッセージを画面表示
                CommandProcess.NotifyMessageToMainUI(AppCommon.MSG_HCHK_CTAP2_LOGIN_TEST_START);
                CommandProcess.NotifyMessageToMainUI(AppCommon.MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT1);
                CommandProcess.NotifyMessageToMainUI(AppCommon.MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT2);
                CommandProcess.NotifyMessageToMainUI(AppCommon.MSG_HCHK_CTAP2_LOGIN_TEST_COMMENT3);
            }

            // GetAssertionコマンドを実行
            switch (Parameter.Transport) {
            case Transport.TRANSPORT_HID:
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestCtapHidCommand(HIDProcessConst.HID_CMD_CTAPHID_CBOR, getAssertionCbor);
                break;
            case Transport.TRANSPORT_BLE:
                CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
                CommandProcess.DoRequestBleCommand(0x80 | FIDO_CMD_MSG, getAssertionCbor);
                break;
            default:
                break;
            }
        }

        private void DoResponseCommandGetAssertion(byte[] message)
        {
            // レスポンスされたCBORを抽出
            byte[] cborBytes = AppUtil.ExtractCBORBytesFromResponse(message, message.Length);

            // GetAssertionレスポンスデータを保持
            //   次のGetAssertionリクエスト送信に必要となる
            //   Saltをhmac-secret拡張情報から抽出して保持
            MakeOrGetCommandResponse commandResponse = CBORDecoder.ParseMakeOrGetCommandResponse(cborBytes, false);

            // GetAssertion実行が２回目かどうか判定
            bool verifySaltNeeded = (HCheckParameter.GetAssertionCount == 2);

            // Saltの検証
            byte[] encryptedSalt = commandResponse.HmacSecretRes.Output;
            if (VerifyHmacSecretSalt(encryptedSalt, verifySaltNeeded) == false) {
                // Salt検証失敗時は画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_CTAP2_ERR_HMAC_INVALID, false);
                return;
            }

            if (verifySaltNeeded) {
                // ２回目のテストが成功したら画面に制御を戻して終了
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_NONE, true);
                return;
            }

            // GetAssertionコマンドを実行する
            HCheckParameter.GetAssertionCount++;
            DoPrepareCommandGetAssertion();
        }

        private bool VerifyHmacSecretSalt(byte[] encryptedSalt, bool verifySaltNeeded)
        {
            // レスポンス内に"hmac-secret"拡張が含まれていない場合はここで終了
            if (encryptedSalt == null) {
                return true;
            }

            if (verifySaltNeeded) {
                // ２回目のGetAssertionの場合はオリジナルSaltと内容を比較し、
                // 同じ内容であれば検証成功
                byte[] decryptedSaltCur = CTAP2Util.AES256CBCDecrypt(HCheckParameter.SharedSecretKey, encryptedSalt);
                bool success = AppUtil.CompareBytes(decryptedSaltCur, HCheckParameter.DecryptedSaltOrg, ExtHmacSecretResponse.OutputSize);

                // 検証結果はログファイル出力する
                AppLogUtil.OutputLogDebug(string.Format(
                    "authenticatorGetAssertion: hmac-secret-salt verify {0}", success ? "success" : "failed")
                    );
                return success;

            } else {
                // １回目のGetAssertionの場合はオリジナルSaltを抽出して終了
                HCheckParameter.DecryptedSaltOrg = CTAP2Util.AES256CBCDecrypt(HCheckParameter.SharedSecretKey, encryptedSalt);
                return true;
            }
        }

        //
        // CBORコマンド関連処理
        //
        private void DoResponseCtapHidCbor(byte[] message)
        {
            // ステータスバイトをチェック
            string errorMessage;
            if (CTAP2Util.CheckStatusByte(message, out errorMessage) == false) {
                // 処理結果が不正の場合は画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, errorMessage, false);
                return;
            }

            switch (HCheckParameter.CborCommand) {
            case CTAP2_CBORCMD_CLIENT_PIN:
                DoResponseCommandClientPin(message);
                break;
            case CTAP2_CBORCMD_MAKE_CREDENTIAL:
                DoResponseCommandMakeCredential(message);
                break;
            case CTAP2_CBORCMD_GET_ASSERTION:
                DoResponseCommandGetAssertion(message);
                break;
            default:
                break;
            }
        }

        private void DoResponseCommandClientPin(byte[] message)
        {
            // レスポンスされたCBORを抽出
            byte[] cborBytes = AppUtil.ExtractCBORBytesFromResponse(message, message.Length);

            switch (HCheckParameter.CborSubCommand) {
            case CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT:
                DoResponseCommandGetKeyAgreement(cborBytes);
                break;
            case CTAP2_SUBCMD_CLIENT_PIN_GET_PIN_TOKEN:
                DoResponseCommandGetPinToken(cborBytes);
                break;
            default:
                break;
            }
        }

        //
        // HID／BLEからのレスポンス振分け処理
        //
        private void OnCommandResponse(byte CMD, byte[] responseData, bool success, string errorMessage)
        {
            // イベントを解除
            CommandProcess.UnregisterHandlerOnCommandResponse(OnCommandResponseRef);

            // 即時でアプリケーションに制御を戻す
            if (success == false) {
                NotifyCommandTerminated(Parameter.CommandTitle, errorMessage, success);
                return;
            }

            // INITからの戻りの場合
            if (CMD == HIDProcessConst.HID_CMD_CTAPHID_INIT) {
                DoResponseCtapHidInit();
                return;
            }
            // GetAgreementからの戻りの場合
            if (CMD == HIDProcessConst.HID_CMD_CTAPHID_CBOR || CMD == (0x80 | FIDO_CMD_MSG)) {
                DoResponseCtapHidCbor(responseData);
                return;
            }

            // 実行コマンドにより処理分岐
            switch (Parameter.Command) {
            default:
                // メイン画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
                break;
            }
        }
    }

    internal class CTAP2HealthCheckTestData
    {
        //
        // ヘルスチェック実行用のテストデータ
        // 
        // MakeCredentialパラメーター
        public MakeCredentialParameter MakeCredentialParameter { get; set; }

        public byte[] Challenge = Encoding.ASCII.GetBytes("This is challenge");

        // hmac-secret機能で使用するsaltを保持
        private readonly Random random = new Random();
        public readonly byte[] Salt = new byte[64];

        public CTAP2HealthCheckTestData()
        {
            // hmac-secret機能で使用する64バイト salt（ランダム値）を生成しておく
            random.NextBytes(Salt);

            // テストデータ項目を設定
            byte[] UserId = Encoding.ASCII.GetBytes("1234567890123456");
            MakeCredentialParameter = new MakeCredentialParameter(
                "diverta.co.jp", "Diverta inc.", UserId, "username", "User Name");
        }
    }
}
