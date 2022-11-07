using System;
using System.Linq;
using System.Text;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;
using static MaintenanceToolApp.OpenPGP.Gpg4winParameter;

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

        private void DoRequestCardEditPasswdCommand()
        {
            // スクリプトを作業用フォルダーに生成
            string scriptName = "card_edit_passwd.bat";
            if (Gpg4winScriptUtil.WriteScriptToTempFolder(scriptName, Parameter) == false) {
                // エラー発生時は、作業用フォルダー消去処理に移行
                OnCommandResponse(false, string.Format(AppCommon.MSG_FORMAT_OPENPGP_CARD_EDIT_PASSWD_GEN_BAT_ERR, Parameter.CommandTitle));
                return;
            }

            // パラメーターファイル名を設定
            string paramName;
            GPGCommand command;
            if (Parameter.Command == Command.COMMAND_OPENPGP_UNBLOCK) {
                paramName = "card_edit_unblock.param";
                command = GPGCommand.COMMAND_GPG_CARD_EDIT_UNBLOCK;
            } else {
                paramName = "card_edit_passwd.param";
                command = GPGCommand.COMMAND_GPG_CARD_EDIT_PASSWD;
            }

            // パラメーターファイルを作業用フォルダーに生成
            if (Gpg4winScriptUtil.WriteParamForCardEditUnblockToTempFolder(paramName, Parameter) == false) {
                // エラー発生時は、作業用フォルダー消去処理に移行
                OnCommandResponse(false, string.Format(AppCommon.MSG_FORMAT_OPENPGP_CARD_EDIT_PASSWD_GEN_PAR_ERR, Parameter.CommandTitle));
                return;
            }

            // スクリプトを実行
            string exe = string.Format("{0}\\{1}", Parameter.TempFolderPath, scriptName);
            string args = string.Format("{0} {1} --no-tty", Parameter.TempFolderPath, paramName);
            Gpg4winParameter param = new Gpg4winParameter(command, exe, args, Parameter.TempFolderPath);
            new Gpg4winProcess().DoRequestCommandLine(param, DoResponseCardEditPasswdCommand);
        }

        private void DoResponseCardEditPasswdCommand(bool success, string response, string error)
        {
            // 変数を初期化
            bool commandSuccess = false;
            string errorMessage = AppCommon.MSG_NONE;

            // レスポンスをチェック
            if (success == false) {
                // スクリプトエラーの場合はOpenPGP cardエラーをチェック
                if (Gpg4winUtility.CheckIfCardErrorFromResponse(error)) {
                    errorMessage = AppCommon.MSG_ERROR_OPENPGP_SELECTING_CARD_FAIL;
                } else {
                    errorMessage = string.Format(AppCommon.MSG_FORMAT_OPENPGP_CARD_EDIT_PASSWD_ERR, Parameter.CommandTitle);
                }

            } else {
                // 成功 or 失敗メッセージが出力されているかどうかチェック
                if (Gpg4winUtility.CheckIfOperationSuccess(response)) {
                    commandSuccess = true;
                } else {
                    string itemName = ItemNameForCardEditPasswdCommand();
                    errorMessage = string.Format(AppCommon.MSG_FORMAT_OPENPGP_CARD_EDIT_PASSWD_NG, itemName);
                }
            }

            // 作業用フォルダー消去処理に移行
            OnCommandResponse(commandSuccess, errorMessage);
        }

        private string ItemNameForCardEditPasswdCommand()
        {
            switch (Parameter.Command) {
            case Command.COMMAND_OPENPGP_UNBLOCK_PIN:
            case Command.COMMAND_OPENPGP_CHANGE_ADMIN_PIN:
            case Command.COMMAND_OPENPGP_SET_RESET_CODE:
                return AppCommon.MSG_LABEL_ITEM_PGP_ADMIN_PIN;
            case Command.COMMAND_OPENPGP_UNBLOCK:
                return AppCommon.MSG_LABEL_ITEM_PGP_RESET_CODE;
            default:
                return AppCommon.MSG_LABEL_ITEM_PGP_PIN;
            }
        }
    }
}
