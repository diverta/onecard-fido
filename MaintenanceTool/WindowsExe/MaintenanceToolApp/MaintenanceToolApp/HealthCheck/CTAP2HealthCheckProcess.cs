using MaintenanceToolApp.Common;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;
using static MaintenanceToolApp.FIDODefine;

namespace MaintenanceToolApp.HealthCheck
{
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

            // getKeyAgreementサブコマンドから実行
            DoRequestCommandGetKeyAgreement();
        }

        public void DoRequestHidCtap2HealthCheck(HandlerOnNotifyCommandTerminated handler)
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
            case Command.COMMAND_HID_CTAP2_HCHECK:
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
            CborCommand = CTAP2_CBORCMD_CLIENT_PIN;
            CborSubCommand = CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT;

            // GetAgreementコマンドバイトを生成
            CBOREncoder cborEncoder = new CBOREncoder();
            byte[] getAgreementCbor = cborEncoder.GetKeyAgreement(CborCommand, CborSubCommand);

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

        private void DoResponseCommandGetKeyAgreement(byte[] responseData)
        {
            string dump = AppLogUtil.DumpMessage(responseData, responseData.Length);
            AppLogUtil.OutputLogDebug(dump);

            // TODO: 仮の実装です。
            NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED, false);
        }

        //
        // CBORコマンド関連処理
        //
        // 実行中のサブコマンドを保持
        private byte CborCommand;
        private byte CborSubCommand;

        public void DoResponseCtapHidCbor(byte[] message)
        {
            // ステータスバイトをチェック
            string errorMessage;
            if (CheckStatusByte(message, out errorMessage) == false) {
                // 処理結果が不正の場合は画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, errorMessage, false);
                return;
            }

            switch (CborCommand) {
            case CTAP2_CBORCMD_CLIENT_PIN:
                DoResponseCommandClientPin(message);
                break;
            default:
                // 正しくレスポンスされなかったと判断し、画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
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
                break;
            }
            return false;
        }

        private void DoResponseCommandClientPin(byte[] message)
        {
            // レスポンスされたCBORを抽出
            byte[] cborBytes = AppUtil.ExtractCBORBytesFromResponse(message, message.Length);

            switch (CborSubCommand) {
            case CTAP2_SUBCMD_CLIENT_PIN_GET_AGREEMENT:
                DoResponseCommandGetKeyAgreement(cborBytes);
                break;
            default:
                // 正しくレスポンスされなかったと判断し、画面に制御を戻す
                NotifyCommandTerminated(Parameter.CommandTitle, AppCommon.MSG_OCCUR_UNKNOWN_ERROR, false);
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
}
