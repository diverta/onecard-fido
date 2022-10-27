using static MaintenanceToolApp.OpenPGP.Gpg4winParameter;

namespace MaintenanceToolApp.OpenPGP
{
    internal class OpenPGPUtilityProcess
    {
        // 処理実行のためのプロパティー
        private OpenPGPParameter Parameter = null!;

        // 上位クラスに対するコールバックを保持
        public delegate void HandlerOnCommandResponse(bool success, string errorMessage);
        private HandlerOnCommandResponse OnCommandResponse = null!;

        public void DoCardStatusCommand(OpenPGPParameter parameterRef, HandlerOnCommandResponse handlerRef)
        {
            // パラメーターを保持
            Parameter = parameterRef;

            // コールバックを保持
            OnCommandResponse = handlerRef;

            // 設定情報照会を実行
            DoRequestCardStatus();
        }

        private void DoRequestCardStatus()
        {
            // スクリプトを作業用フォルダーに生成
            string scriptName = "card_status.bat";
            if (Gpg4winScriptUtil.WriteScriptToTempFolder(scriptName, Parameter) == false) {
                // エラー発生時は、作業用フォルダー消去処理に移行
                OnCommandResponse(false, AppCommon.MSG_ERROR_OPENPGP_STATUS_COMMAND_GEN_BAT);
                return;
            }

            // スクリプトを実行
            string exe = string.Format("{0}\\{1}", Parameter.TempFolderPath, scriptName);
            Gpg4winParameter param = new Gpg4winParameter(GPGCommand.COMMAND_GPG_CARD_STATUS, exe, "", Parameter.TempFolderPath);
            new Gpg4winProcess().DoRequestCommandLine(param, DoResponseCardStatus);
        }

        private void DoResponseCardStatus(bool success, string response, string error)
        {
            // 変数を初期化
            bool commandSuccess = false;
            string errorMessage = AppCommon.MSG_NONE;

            // レスポンスをチェック
            if (success) {
                // レスポンスを保持
                Parameter.StatusInfoString = response;
                commandSuccess = true;

            } else {
                // スクリプトエラーの場合はOpenPGP cardエラーをチェック
                if (Gpg4winUtility.CheckIfCardErrorFromResponse(error)) {
                    errorMessage = AppCommon.MSG_ERROR_OPENPGP_SELECTING_CARD_FAIL;
                } else {
                    errorMessage = AppCommon.MSG_ERROR_OPENPGP_STATUS_COMMAND_FAIL;
                }
            }

            // 作業用フォルダー消去処理に移行
            OnCommandResponse(commandSuccess, errorMessage);
        }
    }
}
