using ToolAppCommon;
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
            Gpg4winParameter parameter = new Gpg4winParameter(GPGCommand.COMMAND_GPG_VERSION, exe, args, null!);
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
            // TODO: 仮の実装です。
            OnCommandResponse(true, AppCommon.MSG_NONE);
        }
    }
}
