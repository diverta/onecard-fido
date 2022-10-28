﻿using static MaintenanceToolApp.AppDefine;
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

            // Gpg4winの`card edit passwd/unblock`コマンドを実行
            DoRequestCardEditPasswdCommand();
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
            // TODO: 仮の実装です。
            OnCommandResponse(true, AppCommon.MSG_NONE);
        }
    }
}
