﻿using ToolAppCommon;
using static MaintenanceToolApp.OpenPGP.Gpg4winParameter;

namespace MaintenanceToolApp.OpenPGP
{
    internal class OpenPGPInstallKeysProcess
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

            // 主鍵生成処理から実行
            DoRequestGenerateMainKey();
        }

        private void DoRequestGenerateMainKey()
        {
            // スクリプトを作業用フォルダーに生成
            string scriptName = "generate_main_key.bat";
            if (Gpg4winProcess.WriteScriptToTempFolder(scriptName, Parameter) == false) {
                // エラー発生時は、作業用フォルダー消去処理に移行
                OnCommandResponse(false, AppCommon.MSG_ERROR_OPENPGP_GENERATE_MAINKEY_GEN_BAT);
                return;
            }

            // パラメーターファイルを作業用フォルダーに生成
            string paramName = "generate_main_key.param";
            if (Gpg4winProcess.WriteParamForGenerateMainKeyToTempFolder(paramName, Parameter) == false) {
                // エラー発生時は、作業用フォルダー消去処理に移行
                OnCommandResponse(false, AppCommon.MSG_ERROR_OPENPGP_GENERATE_MAINKEY_GEN_PAR);
                return;
            }

            // スクリプトを実行し、主鍵を生成
            string exe = string.Format("{0}\\{1}", Parameter.TempFolderPath, scriptName);
            string args = string.Format("{0} {1} --no-tty", Parameter.TempFolderPath, Parameter.Passphrase);
            Gpg4winParameter parameter = new Gpg4winParameter(GPGCommand.COMMAND_GPG_GENERATE_MAIN_KEY, exe, args, null!);
            new Gpg4winProcess().DoRequestCommandLine(parameter, DoResponseGenerateMainKey);
        }

        private void DoResponseGenerateMainKey(bool success, string response, string error)
        {
            // レスポンス内容をチェック
            if (Gpg4winUtility.CheckResponseOfScript(response)) {
                // 生成鍵がCertify機能を有しているかチェック
                string keyid = Gpg4winUtility.ExtractMainKeyIdFromResponse(response);
                if (keyid != string.Empty) {
                    // チェックOKの場合は鍵IDを保持し、次の処理に移行
                    Parameter.GeneratedMainKeyId = keyid;
                    AppLogUtil.OutputLogDebug(string.Format(AppCommon.MSG_FORMAT_OPENPGP_GENERATED_MAIN_KEY, Parameter.GeneratedMainKeyId));
                    DoRequestAddSubKey();
                    return;
                }
            }

            // エラーメッセージを設定し、作業用フォルダー消去処理に移行
            OnCommandResponse(false, AppCommon.MSG_ERROR_OPENPGP_GENERATE_MAINKEY_FAIL);
        }

        private void DoRequestAddSubKey()
        {
            // スクリプトを作業用フォルダーに生成
            string scriptName = "add_sub_key.bat";
            if (Gpg4winProcess.WriteScriptToTempFolder(scriptName, Parameter) == false) {
                // エラー発生時は、作業用フォルダー消去処理に移行
                OnCommandResponse(false, AppCommon.MSG_ERROR_OPENPGP_GENERATE_SUB_KEY_GEN_BAT);
                return;
            }

            // パラメーターファイルを作業用フォルダーに生成
            string paramName = "add_sub_key.param";
            if (Gpg4winProcess.WriteScriptToTempFolder(paramName, Parameter) == false) {
                // エラー発生時は、作業用フォルダー消去処理に移行
                OnCommandResponse(false, AppCommon.MSG_ERROR_OPENPGP_GENERATE_SUB_KEY_GEN_PAR);
                return;
            }

            // スクリプトを実行
            string exe = string.Format("{0}\\{1}", Parameter.TempFolderPath, scriptName);
            string args = string.Format("{0} {1} {2} --no-tty", Parameter.TempFolderPath, Parameter.Passphrase, Parameter.GeneratedMainKeyId);
            Gpg4winParameter parameter = new Gpg4winParameter(GPGCommand.COMMAND_GPG_ADD_SUB_KEY, exe, args, Parameter.TempFolderPath);
            new Gpg4winProcess().DoRequestCommandLine(parameter, DoResponseAddSubKey);
        }

        private void DoResponseAddSubKey(bool success, string response, string error)
        {
            // レスポンス内容をチェック
            if (Gpg4winUtility.CheckResponseOfScript(response)) {
                if (Gpg4winUtility.CheckIfSubKeysExistFromResponse(response, false, Parameter.TempFolderPath)) {
                    // 副鍵が３点生成された場合は、次の処理に移行
                    AppLogUtil.OutputLogDebug(AppCommon.MSG_OPENPGP_ADDED_SUB_KEYS);
                    DoRequestExportPubkeyAndBackup();
                    return;
                }
            }

            // エラーメッセージを設定し、作業用フォルダー消去処理に移行
            OnCommandResponse(false, AppCommon.MSG_ERROR_OPENPGP_GENERATE_SUB_KEY_FAIL);
        }

        private void DoRequestExportPubkeyAndBackup()
        {
            // スクリプトを作業用フォルダーに生成
            string scriptName = "export_pubkey_and_backup.bat";
            if (Gpg4winProcess.WriteScriptToTempFolder(scriptName, Parameter) == false) {
                // エラー発生時は、作業用フォルダー消去処理に移行
                OnCommandResponse(false, AppCommon.MSG_ERROR_OPENPGP_EXPORT_BACKUP_GEN_BAT);
                return;
            }

            // スクリプトを実行
            string exe = string.Format("{0}\\{1}", Parameter.TempFolderPath, scriptName);
            string args = string.Format("{0} {1} {2} {3} {4}", Parameter.TempFolderPath, Parameter.Passphrase, Parameter.GeneratedMainKeyId,
                Parameter.PubkeyFolderPath, Parameter.BackupFolderPath);
            Gpg4winParameter parameter = new Gpg4winParameter(GPGCommand.COMMAND_GPG_EXPORT_PUBKEY_AND_BACKUP, exe, args, Parameter.TempFolderPath);
            new Gpg4winProcess().DoRequestCommandLine(parameter, DoResponseExportPubkeyAndBackup);
        }

        private void DoResponseExportPubkeyAndBackup(bool success, string response, string error)
        {
            // レスポンス内容をチェック
            if (Gpg4winUtility.CheckResponseOfScript(response)) {
                if (Gpg4winUtility.CheckIfPubkeyAndBackupExist(Parameter.PubkeyFolderPath, Parameter.BackupFolderPath)) {
                    // 公開鍵ファイル、バックアップファイルが生成された場合は、次の処理に移行
                    AppLogUtil.OutputLogDebug(string.Format(AppCommon.MSG_FORMAT_OPENPGP_EXPORT_PUBKEY_DONE, Parameter.PubkeyFolderPath));
                    AppLogUtil.OutputLogDebug(string.Format(AppCommon.MSG_FORMAT_OPENPGP_EXPORT_BACKUP_DONE, Parameter.BackupFolderPath));
                    DoRequestTransferSubkeyToCard();
                    return;
                }
            }

            // エラーメッセージを設定し、作業用フォルダー消去処理に移行
            OnCommandResponse(false, AppCommon.MSG_ERROR_OPENPGP_EXPORT_BACKUP_FAIL);
        }

        private void DoRequestTransferSubkeyToCard()
        {
            // TODO: 仮の実装です。
            OnCommandResponse(true, AppCommon.MSG_NONE);
        }
    }
}
