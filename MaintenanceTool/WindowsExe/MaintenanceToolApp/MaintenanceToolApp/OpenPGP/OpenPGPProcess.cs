using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;
using static MaintenanceToolApp.OpenPGP.Gpg4winParameter;

namespace MaintenanceToolApp.OpenPGP
{
    public class OpenPGPParameter
    {
        public string CommandTitle { get; set; }
        public Command Command { get; set; }
        public bool CommandSuccess { get; set; }
        public string ResultMessage { get; set; }
        public string ResultInformativeMessage { get; set; }
        public string RealName { get; set; }
        public string MailAddress { get; set; }
        public string Comment { get; set; }
        public string Passphrase { get; set; }
        public string PubkeyFolderPath { get; set; }
        public string BackupFolderPath { get; set; }
        public string CurrentPin { get; set; }
        public string NewPin { get; set; }
        //
        // 以下は処理生成中に設定
        //
        public string TempFolderPath { get; set; }
        public string GeneratedMainKeyId { get; set; }

        public OpenPGPParameter()
        {
            CommandTitle = string.Empty;
            Command = Command.COMMAND_NONE;
            ResultMessage = string.Empty;
            ResultInformativeMessage = string.Empty;
            RealName = string.Empty;
            MailAddress = string.Empty;
            Comment = string.Empty;
            Passphrase = string.Empty;
            PubkeyFolderPath = string.Empty;
            BackupFolderPath = string.Empty;
            CurrentPin = string.Empty;
            NewPin = string.Empty;
            TempFolderPath = string.Empty;
            GeneratedMainKeyId = string.Empty;
        }

        public override string ToString()
        {
            string command = string.Format("Command:{0} CommandTitle:{1}", Command, CommandTitle);
            string PGPKeyParam = string.Format("RealName:{0} MailAddress:{1} Comment:{2} Passphrase:{3} PubkeyFolderPath:{4} BackupFolderPath:{5}",
                RealName, MailAddress, Comment, Passphrase, PubkeyFolderPath, BackupFolderPath);
            string PinCommandParam = string.Format("CurrentPin:{0} NewPin:{1}", CurrentPin, NewPin);
            return string.Format("{0}\n{1}\n{2}", command, PGPKeyParam, PinCommandParam);
        }
    }

    public class OpenPGPProcess
    {
        // 処理実行のためのプロパティー
        private OpenPGPParameter Parameter = null!;

        // 上位クラスに対するイベント通知
        public delegate void HandlerOnNotifyProcessTerminated(OpenPGPParameter parameter);
        private event HandlerOnNotifyProcessTerminated OnNotifyProcessTerminated = null!;

        // イベントのコールバック参照
        private HandlerOnNotifyProcessTerminated OnNotifyProcessTerminatedRef = null!;

        // CCIDコマンド処理クラスのインスタンスを保持
        private readonly OpenPGPCCIDProcess CCIDProcess = new OpenPGPCCIDProcess();

        // Gpg4winコマンド処理クラスのインスタンスを保持
        private readonly Gpg4winProcess GpgProcess = new Gpg4winProcess();

        //
        // OpenPGP機能設定用関数
        // 
        public void DoOpenPGPProcess(OpenPGPParameter parameter, HandlerOnNotifyProcessTerminated handlerRef)
        {
            // 画面から引き渡されたパラメーターを退避
            Parameter = parameter;

            // コールバックを登録
            OnNotifyProcessTerminatedRef = handlerRef;
            OnNotifyProcessTerminated += OnNotifyProcessTerminatedRef;

            // 処理開始を通知
            NotifyProcessStarted();

            if (Parameter.Command == Command.COMMAND_OPENPGP_INSTALL_KEYS) {
                // 管理用PIN番号検証から開始
                DoRequestAdminPinVerify();

            } else {
                // バージョン照会から開始
                DoRequestGPGVersion();
            }
        }

        //
        // CCID I/Fコマンド実行関数
        //
        private void DoRequestAdminPinVerify()
        {
            // 事前にCCID I/F経由で、管理用PIN番号の検証を試行
            CCIDProcess.DoOpenPGPCcidCommand(Parameter, DoResponseAdminPinVerify);
        }

        private void DoResponseAdminPinVerify(bool success, string errorMessage)
        {
            if (success) {
                // バージョン照会から開始
                AppLogUtil.OutputLogDebug(AppCommon.MSG_OPENPGP_ADMIN_PIN_VERIFIED);
                DoRequestGPGVersion();

            } else {
                // 画面に制御を戻す
                NotifyProcessTerminated(false, errorMessage);
            }
        }

        //
        // GPGコマンド実行関数
        // 
        private void DoRequestGPGVersion()
        {
            Gpg4winParameter parameter = new Gpg4winParameter(GPGCommand.COMMAND_GPG_VERSION, "gpg", "--version", null!);
            GpgProcess.DoRequestCommandLine(parameter, DoResponseGPGVersion);
        }

        private void DoResponseGPGVersion(bool success, string response, string error)
        {
            // PCに導入されているGPGが、所定のバージョン以上でない場合は終了
            if (success == false || Gpg4winUtility.CheckIfGPGVersionAvailable(response) == false) {
                // 画面に制御を戻す
                NotifyProcessTerminated(false, AppCommon.MSG_ERROR_OPENPGP_GPG_VERSION_UNAVAIL);
                return;
            }

            // 次の処理に移行
            DoRequestCreateTempFolder();
        }

        //
        // 作業フォルダー生成／消去
        //
        private void DoRequestCreateTempFolder()
        {
            // 作業用フォルダーをPC上に生成
            GpgProcess.MakeTempFolder(DoResponseCreateTempFolder);
        }

        private void DoResponseCreateTempFolder(bool success, string createdTempFolderPath)
        {
            // レスポンスをチェック
            if (success == false) {
                // 画面に制御を戻す
                NotifyProcessTerminated(false, AppCommon.MSG_ERROR_OPENPGP_CREATE_TEMPDIR_FAIL);
                return;
            }

            // 生成された作業用フォルダー名称を保持
            Parameter.TempFolderPath = createdTempFolderPath;
            AppLogUtil.OutputLogDebug(string.Format(AppCommon.MSG_FORMAT_OPENPGP_CREATED_TEMPDIR, Parameter.TempFolderPath));

            // コマンドに応じ、以下の処理に分岐
            switch (Parameter.Command) {
            case Command.COMMAND_OPENPGP_INSTALL_KEYS:
                DoRequestGenerateMainKey();
                break;
            default:
                NotifyProcessTerminated(false, AppCommon.MSG_OCCUR_UNKNOWN_ERROR);
                break;
            }
        }

        private void DoRequestRemoveTempFolder()
        {
            // 作業用フォルダーをPC上から削除
            GpgProcess.RemoveTempFolder(DoResponseRemoveTempFolder);
        }

        private void DoRequestRemoveTempFolderWithInformative(bool commandSuccess, string resultInformativeMessage)
        {
            // エラーメッセージを設定し、作業用フォルダー消去処理に移行
            Parameter.ResultInformativeMessage = resultInformativeMessage;
            Parameter.CommandSuccess = commandSuccess;
            DoRequestRemoveTempFolder();
        }

        private void DoResponseRemoveTempFolder(bool success, string removedTempFolderPath)
        {
            // レスポンスをチェック
            if (success == false) {
                NotifyProcessTerminated(false, AppCommon.MSG_ERROR_OPENPGP_REMOVE_TEMPDIR_FAIL);
                return;
            }

            // 処理完了を通知
            AppLogUtil.OutputLogDebug(AppCommon.MSG_OPENPGP_REMOVED_TEMPDIR);
            NotifyProcessTerminated(Parameter.CommandSuccess, Parameter.ResultInformativeMessage);
        }
        
        //
        // PGP秘密鍵インストール関連
        //
        private void DoRequestGenerateMainKey()
        {
            // スクリプトを作業用フォルダーに生成
            string scriptName = "generate_main_key.bat";
            if (GpgProcess.WriteScriptToTempFolder(scriptName) == false) {
                // エラー発生時は、作業用フォルダー消去処理に移行
                DoRequestRemoveTempFolderWithInformative(false, AppCommon.MSG_ERROR_OPENPGP_GENERATE_MAINKEY_GEN_BAT);
                return;
            }

            // パラメーターファイルを作業用フォルダーに生成
            string paramName = "generate_main_key.param";
            if (GpgProcess.WriteParamForGenerateMainKeyToTempFolder(paramName, Parameter) == false) {
                // エラー発生時は、作業用フォルダー消去処理に移行
                DoRequestRemoveTempFolderWithInformative(false, AppCommon.MSG_ERROR_OPENPGP_GENERATE_MAINKEY_GEN_PAR);
                return;
            }

            // スクリプトを実行し、主鍵を生成
            string exe = string.Format("{0}\\{1}", Parameter.TempFolderPath, scriptName);
            string args = string.Format("{0} {1} --no-tty", Parameter.TempFolderPath, Parameter.Passphrase);
            Gpg4winParameter parameter = new Gpg4winParameter(GPGCommand.COMMAND_GPG_VERSION, exe, args, null!);
            GpgProcess.DoRequestCommandLine(parameter, DoResponseGenerateMainKey);
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
            DoRequestRemoveTempFolderWithInformative(false, AppCommon.MSG_ERROR_OPENPGP_GENERATE_MAINKEY_FAIL);
        }

        private void DoRequestAddSubKey()
        {
            // TODO: 仮の実装です。
            Parameter.CommandSuccess = true;
            DoRequestRemoveTempFolder();
        }

        // 
        // 共通処理
        //
        private void NotifyProcessStarted()
        {
            // コマンド開始メッセージをログファイルに出力
            string startMsg = string.Format(AppCommon.MSG_FORMAT_START_MESSAGE, Parameter.CommandTitle);
            AppLogUtil.OutputLogInfo(startMsg);
        }

        private void NotifyProcessTerminated(bool success, string errorMessage)
        {
            // エラーメッセージを画面＆ログ出力
            if (success == false && errorMessage.Length > 0) {
                // ログ出力する文言からは、改行文字を除去
                AppLogUtil.OutputLogError(AppUtil.ReplaceCRLF(errorMessage));
                Parameter.ResultInformativeMessage = errorMessage;
            }

            // コマンドの実行結果をログ出力
            string formatted = string.Format(AppCommon.MSG_FORMAT_END_MESSAGE,
                Parameter.CommandTitle,
                success ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE);
            if (success) {
                AppLogUtil.OutputLogInfo(formatted);
            } else {
                AppLogUtil.OutputLogError(formatted);
            }

            // パラメーターにコマンド成否を設定
            Parameter.CommandSuccess = success;
            Parameter.ResultMessage = formatted;

            // 画面に制御を戻す            
            OnNotifyProcessTerminated(Parameter);

            // コールバックを解除
            OnNotifyProcessTerminated -= OnNotifyProcessTerminatedRef;
        }
    }
}
