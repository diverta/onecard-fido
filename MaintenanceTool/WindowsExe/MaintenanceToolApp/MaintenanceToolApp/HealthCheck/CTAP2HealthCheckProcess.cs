using MaintenanceToolApp.Common;
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
        public CTAP2HealthCheckTestData TestData = null!;

        public CTAP2HealthCheckParameter()
        {
            AgreementPublicKey = new KeyAgreement();
            SharedSecretKey = new byte[0];
            PinHashEnc = new byte[0];
        }
    }

    internal class CTAP2HealthCheckProcess
    {
        // 処理実行のためのプロパティー
        private readonly HealthCheckParameter Parameter;

        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyCommandTerminated(string commandTitle, string errorMessage, bool success);
        private event HandlerOnNotifyCommandTerminated NotifyCommandTerminated = null!;

        // HIDインターフェースからデータ受信時のコールバック参照
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
                CommandProcess.DoRequestBleCommand(U2FProcessConst.U2F_CMD_MSG, getAgreementCbor);
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
            CTAP2Util util = new CTAP2Util();
            if (util.GenerateSharedSecretKey(HCheckParameter) == false) {
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_CTAP2_ERR_PIN_AUTH_SSKEY_GENERATE, false);
                return;
            }

            // PinHashEncを生成
            util.GeneratePinHashEnc(Parameter.Pin, HCheckParameter);

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
                CommandProcess.DoRequestBleCommand(U2FProcessConst.U2F_CMD_MSG, getPinTokenCbor);
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
            byte[] pinToken = CTAP2Util.AES256CBCDecrypt(pinTokenEnc, HCheckParameter.SharedSecretKey);

            // for debug
            AppLogUtil.OutputLogDebug(string.Format(
                "pinToken: \n{0}", AppLogUtil.DumpMessage(pinToken, pinToken.Length)));

            switch (Parameter.Command) {
            case Command.COMMAND_TEST_MAKE_CREDENTIAL:
                // MakeCredentialコマンドを実行
                DoRequestCommandMakeCredential(pinToken);
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
                CommandProcess.DoRequestBleCommand(U2FProcessConst.U2F_CMD_MSG, makeCredentialCbor);
                break;
            default:
                break;
            }
        }

        private void DoResponseCommandMakeCredential(byte[] cborBytes)
        {
            // TODO: 仮の実装です。
            NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED, false);
        }

        //
        // CBORコマンド関連処理
        //
        private void DoResponseCtapHidCbor(byte[] message)
        {
            // ステータスバイトをチェック
            string errorMessage;
            if (CheckStatusByte(message, out errorMessage) == false) {
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
            default:
                break;
            }
        }

        private bool CheckStatusByte(byte[] receivedMessage, out string errorMessage)
        {
            errorMessage = "";
            switch (receivedMessage[0]) {
            case CTAP1_ERR_SUCCESS:
                return true;
            case CTAP2_ERR_PIN_INVALID:
            case CTAP2_ERR_PIN_AUTH_INVALID:
                errorMessage = AppCommon.MSG_CTAP2_ERR_PIN_INVALID;
                break;
            case CTAP2_ERR_PIN_BLOCKED:
                errorMessage = AppCommon.MSG_CTAP2_ERR_PIN_BLOCKED;
                break;
            case CTAP2_ERR_PIN_AUTH_BLOCKED:
                errorMessage = AppCommon.MSG_CTAP2_ERR_PIN_AUTH_BLOCKED;
                break;
            case CTAP2_ERR_PIN_NOT_SET:
                errorMessage = AppCommon.MSG_CTAP2_ERR_PIN_NOT_SET;
                break;
            case CTAP2_ERR_VENDOR_KEY_CRT_NOT_EXIST: // CTAP2_ERR_VENDOR_FIRST+0x0e
                errorMessage = AppCommon.MSG_OCCUR_SKEYNOEXIST_ERROR;
                break;
            default:
                errorMessage = string.Format("不明なステータスにより処理が失敗しました: 0x{0:x2}", receivedMessage[0]) ;
                break;
            }
            return false;
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
            if (CMD == HIDProcessConst.HID_CMD_CTAPHID_CBOR || CMD == U2FProcessConst.U2F_CMD_MSG) {
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
