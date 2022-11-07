using System;
using System.Linq;
using System.Text;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.OpenPGP
{
    internal class OpenPGPPinManagementProcess
    {
        // 処理実行のためのプロパティー
        private OpenPGPParameter Parameter = null!;

        // 上位クラスに対するコールバックを保持
        public delegate void HandlerOnCommandResponse(bool success, string errorMessage);
        private HandlerOnCommandResponse OnCommandResponse = null!;

        public void DoProcess(OpenPGPParameter parameterRef, HandlerOnCommandResponse handlerRef)
        {
            // パラメーターを保持
            Parameter = parameterRef;

            // コールバックを保持
            OnCommandResponse = handlerRef;

            // コマンドに応じ、以下の処理に分岐
            switch (Parameter.Command) {
            case Command.COMMAND_OPENPGP_CHANGE_PIN:
                DoRequestChangePin();
                break;
            case Command.COMMAND_OPENPGP_CHANGE_ADMIN_PIN:
                DoRequestChangeAdminPin();
                break;
            case Command.COMMAND_OPENPGP_UNBLOCK_PIN:
                DoRequestUnblockPin();
                break;
            case Command.COMMAND_OPENPGP_SET_RESET_CODE:
                DoRequestSetResetCode();
                break;
            case Command.COMMAND_OPENPGP_UNBLOCK:
                DoRequestUnblock();
                break;
            default:
                // 上位クラスに制御を戻す
                OnCommandResponse(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                break;
            }
        }

        //
        // PIN番号を変更
        //
        private void DoRequestChangePin()
        {
            // パラメーターチェック
            int digit = 6;
            if (Parameter.CurrentPin.Length != digit || Parameter.NewPin.Length != digit) {
                string errorMessage = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_PIN_DIGIT, AppCommon.MSG_LABEL_ITEM_PGP_PIN, digit);
                OnCommandResponse(false, errorMessage);
                return;
            }

            // パラメーターを生成
            byte[] paramPinBytes = GenerateParamPinBytes(Parameter.CurrentPin, Parameter.NewPin);

            // PIN番号の変更を実行
            CCIDParameter param = new CCIDParameter(OpenPGPCCIDConst.OPENPGP_INS_CHANGE_REFERENCE_DATA, 0x00, 0x81, paramPinBytes, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponseChangePin);
        }

        private void DoResponseChangePin(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 上位クラスに制御を戻す
            DoResponsePinCommand(success, responseData, responseSW);
        }

        //
        // 管理用PIN番号を変更
        //
        private void DoRequestChangeAdminPin()
        {
            // パラメーターチェック
            int digit = 8;
            if (Parameter.CurrentPin.Length != digit || Parameter.NewPin.Length != digit) {
                string errorMessage = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_PIN_DIGIT, AppCommon.MSG_LABEL_ITEM_PGP_ADMIN_PIN, digit);
                OnCommandResponse(false, errorMessage);
                return;
            }

            // パラメーターを生成
            byte[] paramPinBytes = GenerateParamPinBytes(Parameter.CurrentPin, Parameter.NewPin);

            // PIN番号の変更を実行
            CCIDParameter param = new CCIDParameter(OpenPGPCCIDConst.OPENPGP_INS_CHANGE_REFERENCE_DATA, 0x00, 0x83, paramPinBytes, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponseChangeAdminPin);
        }

        private void DoResponseChangeAdminPin(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 上位クラスに制御を戻す
            DoResponsePinCommand(success, responseData, responseSW);
        }

        //
        // PIN番号をリセット
        //
        private void DoRequestUnblockPin()
        {
            // パラメーターチェック
            int digitPin = 6;
            if (Parameter.NewPin.Length != digitPin) {
                string errorMessage = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_PIN_DIGIT, AppCommon.MSG_LABEL_ITEM_PGP_PIN, digitPin);
                OnCommandResponse(false, errorMessage);
                return;
            }

            // パラメーターを生成
            byte[] paramPinBytes = GenerateParamPinBytes(Parameter.NewPin, string.Empty);

            // PIN番号のリセットを実行
            CCIDParameter param = new CCIDParameter(OpenPGPCCIDConst.OPENPGP_INS_RESET_RETRY_COUNTER, 0x02, 0x81, paramPinBytes, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponseUnblockPin);
        }

        private void DoResponseUnblockPin(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (success == false || responseSW != CCIDProcessConst.SW_SUCCESS) {
                OnCommandResponse(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                return;
            }

            // 上位クラスに制御を戻す
            OnCommandResponse(true, AppCommon.MSG_NONE);
        }

        //
        // リセットコードを変更
        //
        private void DoRequestSetResetCode()
        {
            // パラメーターチェック
            int digitRc = 8;
            if (Parameter.NewPin.Length != digitRc) {
                string errorMessage = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_PIN_DIGIT, AppCommon.MSG_LABEL_ITEM_PGP_RESET_CODE, digitRc);
                OnCommandResponse(false, errorMessage);
                return;
            }

            // パラメーターを生成
            byte[] paramPinBytes = GenerateParamPinBytes(Parameter.NewPin, string.Empty);

            // リセットコード変更を実行
            CCIDParameter param = new CCIDParameter(OpenPGPCCIDConst.OPENPGP_INS_PUT_DATA, 0x00, 0xd3, paramPinBytes, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponseSetResetCode);
        }

        private void DoResponseSetResetCode(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (success == false || responseSW != CCIDProcessConst.SW_SUCCESS) {
                OnCommandResponse(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                return;
            }

            // 上位クラスに制御を戻す
            OnCommandResponse(true, AppCommon.MSG_NONE);
        }

        //
        // リセットコードでPIN番号をリセット
        //
        private void DoRequestUnblock()
        {
            // パラメーターチェック
            int digitRc = 8;
            if (Parameter.CurrentPin.Length != digitRc) {
                string errorMessage = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_PIN_DIGIT, AppCommon.MSG_LABEL_ITEM_PGP_RESET_CODE, digitRc);
                OnCommandResponse(false, errorMessage);
                return;
            }
            int digitPin = 6;
            if (Parameter.NewPin.Length != digitPin) {
                string errorMessage = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_PIN_DIGIT, AppCommon.MSG_LABEL_ITEM_PGP_PIN, digitPin);
                OnCommandResponse(false, errorMessage);
                return;
            }

            // パラメーターを生成
            byte[] paramPinBytes = GenerateParamPinBytes(Parameter.CurrentPin, Parameter.NewPin);

            // PIN番号の変更を実行
            CCIDParameter param = new CCIDParameter(OpenPGPCCIDConst.OPENPGP_INS_RESET_RETRY_COUNTER, 0x00, 0x81, paramPinBytes, 0xff);
            CCIDProcess.DoRequestCommand(param, DoResponseUnblock);
        }

        private void DoResponseUnblock(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 上位クラスに制御を戻す
            DoResponsePinCommand(success, responseData, responseSW);
        }

        //
        // 共通処理
        //
        private static byte[] GenerateParamPinBytes(string currentPin, string newPin)
        {
            // パラメーターを生成
            byte[] curPinBytes = Encoding.ASCII.GetBytes(currentPin);
            byte[] newPinBytes = Encoding.ASCII.GetBytes(newPin);
            return curPinBytes.Concat(newPinBytes).ToArray();
        }

        private void DoResponsePinCommand(bool success, byte[] responseData, UInt16 responseSW)
        {
            // 不明なエラーが発生時は以降の処理を行わない
            if (success == false) {
                OnCommandResponse(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                return;
            }

            // 認証が失敗した場合は以降の処理を行わない
            string errorMessage;
            if (OpenPGPCCIDProcess.CheckPinCommandResponseSW(Parameter.Command, responseSW, out errorMessage) == false) {
                OnCommandResponse(false, errorMessage);
                return;
            }

            // 上位クラスに制御を戻す
            OnCommandResponse(true, AppCommon.MSG_NONE);
        }
    }
}
