using MaintenanceToolApp.Common;
using MaintenanceToolApp.HealthCheck;
using System;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.FIDOSettings
{
    internal class SetPinCodeProcess
    {
        // 処理実行のためのプロパティー
        private readonly FIDOSettingsParameter Parameter;
        private readonly CTAP2HealthCheckParameter CTAP2Parameter;

        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyCommandTerminated(string commandTitle, string errorMessage, bool success);
        private event HandlerOnNotifyCommandTerminated NotifyCommandTerminated = null!;

        // HID／BLEからデータ受信時のコールバック参照
        private readonly CommandProcess.HandlerOnCommandResponse OnCommandResponseRef;

        public SetPinCodeProcess(FIDOSettingsParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;

            // CTAP2パラメーターを生成
            CTAP2Parameter = new CTAP2HealthCheckParameter();

            // コールバック参照を初期化
            OnCommandResponseRef = new CommandProcess.HandlerOnCommandResponse(OnCommandResponse);
        }

        //
        // 外部公開用
        //
        public void DoRequestSetPinCode(HandlerOnNotifyCommandTerminated handler)
        {
            // 戻り先の関数を保持
            NotifyCommandTerminated = handler;

            // CTAPHID_INITから実行
            DoRequestCtapHidInit();
        }

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
            case Command.COMMAND_CLIENT_PIN_SET:
            case Command.COMMAND_CLIENT_PIN_CHANGE:
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
            CTAP2Parameter.CborCommand = FIDODefine.CTAP2_CBORCMD_CLIENT_PIN;
            CTAP2Parameter.CborSubCommand = FIDODefine.CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT;

            // GetAgreementコマンドバイトを生成
            CBOREncoder cborEncoder = new CBOREncoder();
            byte[] getAgreementCbor = CBOREncoder.GenerateGetKeyAgreementCbor(CTAP2Parameter.CborCommand, CTAP2Parameter.CborSubCommand);

            // GetAgreementコマンドを実行
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCtapHidCommand(HIDProcessConst.HID_CMD_CTAPHID_CBOR, getAgreementCbor);
        }

        private void DoResponseCommandGetKeyAgreement(byte[] cborBytes)
        {
            // CBORをデコードして公開鍵を抽出
            CBORDecoder cborDecoder = new CBORDecoder();
            CTAP2Parameter.AgreementPublicKey = CBORDecoder.GetKeyAgreement(cborBytes);

            // 共通鍵を生成
            if (CTAP2Util.GenerateSharedSecretKey(CTAP2Parameter) == false) {
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_CTAP2_ERR_SSKEY_GENERATE_FOR_SET_PIN_CODE, false);
                return;
            }

            // PINコード、共通鍵からNewPinEncを生成
            byte[] newPinEnc = CTAP2Util.CreateNewPinEnc(Parameter.PinNew, CTAP2Parameter.SharedSecretKey);

            switch (Parameter.Command) {
            case Command.COMMAND_CLIENT_PIN_SET:
                DoRequestCommandClientPinSet(newPinEnc);
                break;
            case Command.COMMAND_CLIENT_PIN_CHANGE:
                DoRequestCommandClientPinChange(newPinEnc);
                break;
            default:
                // 正しくレスポンスされなかったと判断し、画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
                break;
            }
        }

        //
        // PINコードの設定
        //
        private void DoRequestCommandClientPinSet(byte[] newPinEnc)
        {
            // 実行するコマンドを保持
            CTAP2Parameter.CborCommand = FIDODefine.CTAP2_CBORCMD_CLIENT_PIN;
            CTAP2Parameter.CborSubCommand = FIDODefine.CTAP2_SUBCMD_CLIENT_PIN_SET;

            // NewPinEncと共通鍵から、PinAuthを生成
            byte[] pinAuth = CTAP2Util.CreatePinAuth(newPinEnc, Array.Empty<byte>(), CTAP2Parameter.SharedSecretKey);

            // setPinコマンドバイトを生成
            byte[] setPinCbor = CBOREncoder.GenerateSetPinCbor(
                CTAP2Parameter.CborCommand, CTAP2Parameter.CborSubCommand,
                CTAP2Parameter.AgreementPublicKey, pinAuth, newPinEnc, Array.Empty<byte>());

            // コマンドを実行する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCtapHidCommand(HIDProcessConst.HID_CMD_CTAPHID_CBOR, setPinCbor);
        }

        private void DoRequestCommandClientPinChange(byte[] newPinEnc)
        {
            // 実行するコマンドを保持
            CTAP2Parameter.CborCommand = FIDODefine.CTAP2_CBORCMD_CLIENT_PIN;
            CTAP2Parameter.CborSubCommand = FIDODefine.CTAP2_SUBCMD_CLIENT_PIN_CHANGE;

            // PINコード変更の場合は、pinHashEncを生成
            CTAP2Util.GeneratePinHashEnc(Parameter.PinOld, CTAP2Parameter);

            // NewPinEnc、PinHashEncと共通鍵から、PinAuthを生成
            byte[] pinAuth = CTAP2Util.CreatePinAuth(newPinEnc, CTAP2Parameter.PinHashEnc, CTAP2Parameter.SharedSecretKey);

            // setPinコマンドバイトを生成
            byte[] setPinCbor = CBOREncoder.GenerateSetPinCbor(
                CTAP2Parameter.CborCommand, CTAP2Parameter.CborSubCommand,
                CTAP2Parameter.AgreementPublicKey, pinAuth, newPinEnc, CTAP2Parameter.PinHashEnc);

            // コマンドを実行する
            CommandProcess.RegisterHandlerOnCommandResponse(OnCommandResponseRef);
            CommandProcess.DoRequestCtapHidCommand(HIDProcessConst.HID_CMD_CTAPHID_CBOR, setPinCbor);
        }

        private void DoResponseCommandClientPinSet(byte[] responseData)
        {
            // 上位クラスに制御を戻す
            NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_NONE, true);
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

            switch (CTAP2Parameter.CborCommand) {
            case FIDODefine.CTAP2_CBORCMD_CLIENT_PIN:
                DoResponseCommandClientPin(message);
                break;
            default:
                break;
            }
        }

        private void DoResponseCommandClientPin(byte[] message)
        {
            // レスポンスされたCBORを抽出
            byte[] cborBytes = AppUtil.ExtractCBORBytesFromResponse(message, message.Length);

            switch (CTAP2Parameter.CborSubCommand) {
            case FIDODefine.CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT:
                DoResponseCommandGetKeyAgreement(cborBytes);
                break;
            case FIDODefine.CTAP2_SUBCMD_CLIENT_PIN_SET:
            case FIDODefine.CTAP2_SUBCMD_CLIENT_PIN_CHANGE:
                DoResponseCommandClientPinSet(cborBytes);
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

            // CBORからの戻りの場合
            if (CMD == HIDProcessConst.HID_CMD_CTAPHID_CBOR) {
                DoResponseCtapHidCbor(responseData);
                return;
            }

            // メイン画面に制御を戻す
            NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
        }
    }
}
