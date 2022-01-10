using MaintenanceToolCommon;
using System;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public class ToolPGPParameter
    {
        // 鍵作成用パラメーター
        public string RealName { get; set; }
        public string MailAddress { get; set; }
        public string Comment { get; set; }
        public string Passphrase { get; set; }
        public string PubkeyFolderPath { get; set; }
        public string BackupFolderPath { get; set; }
    }

    public class ToolPGP
    {
        // 画面の参照を保持
        private MainForm mainForm;

        // 処理クラスの参照を保持
        private HIDMain hidMain;

        // 処理機能を保持
        private AppCommon.RequestType RequestType;

        // 処理機能名を保持
        private string NameOfCommand;

        // エラーメッセージテキストを保持
        private string ErrorMessageOfCommand;

        // ステータス照会情報を保持
        private string StatusInfoString;

        // 生成された作業用フォルダー名称を保持
        private string TempFolderPath;

        // 実行する自動認証設定コマンドの種別
        public enum GPGCommand
        {
            COMMAND_GPG_NONE = 1,
            COMMAND_GPG_VERSION,
            COMMAND_GPG_MAKE_TEMP_FOLDER,
            COMMAND_GPG_GENERATE_MAIN_KEY,
            COMMAND_GPG_ADD_SUB_KEY,
            COMMAND_GPG_EXPORT_PUBKEY_AND_BACKUP,
            COMMAND_GPG_TRANSFER_SUBKEY_TO_CARD,
            COMMAND_GPG_REMOVE_TEMP_FOLDER,
            COMMAND_GPG_CARD_STATUS,
            COMMAND_GPG_CARD_RESET,
        };

        // リクエストパラメーターを保持
        private ToolPGPParameter toolPGPParameter = null;

        public ToolPGP(MainForm f, HIDMain h)
        {
            // メイン画面の参照を保持
            mainForm = f;

            // HID処理クラスの参照を保持
            hidMain = h;
        }

        public void ShowDialog()
        {
            // TODO: 仮の実装です。
            ToolPGPParameter parameter = new ToolPGPParameter();
            parameter.RealName = "";
            parameter.MailAddress = "";
            parameter.Comment = "";
            parameter.Passphrase = "";
            parameter.PubkeyFolderPath = "";
            parameter.BackupFolderPath = "";
            DoCommandInstallPGPKey(parameter);
        }

        //
        // OpenPGP機能設定用関数
        // 
        public void DoCommandInstallPGPKey(ToolPGPParameter parameter)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 画面から引き渡されたパラメーターを退避
            toolPGPParameter = parameter;

            // バージョン照会から開始
            NotifyProcessStarted(AppCommon.RequestType.OpenPGPInstallKeys);
            DoRequestGPGVersion();
        }

        public void DoCommandPGPStatus()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }

            // バージョン照会から開始
            NotifyProcessStarted(AppCommon.RequestType.OpenPGPStatus);
            DoRequestGPGVersion();
        }

        //
        // GPGコマンド実行関数
        // 
        private void DoRequestGPGVersion()
        {
            // インストールされているGPGコマンドのバージョンを照会
            DoRequestCommandLine(GPGCommand.COMMAND_GPG_VERSION, "gpg", "--version", null);
        }

        private void DoResponseGPGVersion(bool success, string response)
        {
            // PCに導入されているGPGが、所定のバージョン以上でない場合は終了
            if (success == false || CheckIfGPGVersionAvailable(response) == false) {
                NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_GPG_VERSION_UNAVAIL);
                NotifyProcessTerminated(false);
                return;
            }

            // コマンドに応じ、以下の処理に分岐
            switch (RequestType) {
                case AppCommon.RequestType.OpenPGPStatus:
                    DoRequestCardStatus();
                    break;
                default:
                    // 次の処理に移行
                    DoRequestMakeTempFolder();
                    break;
            }
        }

        private void DoRequestCardStatus()
        {
            // インストールされているGPGコマンドのバージョンを照会
            DoRequestCommandLine(GPGCommand.COMMAND_GPG_CARD_STATUS, "gpg", "--card-status", null);
        }

        private void DoResponseCardStatus(bool success, string response)
        {
            // レスポンスをチェック
            if (success) {
                // レスポンスを保持
                StatusInfoString = response;
                AppCommon.OutputLogInfo(StatusInfoString);

            } else {
                // スクリプトエラーの場合はOpenPGP cardエラーをチェック
                if (CheckIfCardErrorFromResponse(response)) {
                    NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_SELECTING_CARD_FAIL);
                } else {
                    NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_STATUS_COMMAND_FAIL);
                }
            }

            // 処理完了を通知
            NotifyProcessTerminated(success);
        }

        private void DoRequestMakeTempFolder()
        {
            // 作業用フォルダーをPC上に生成
            MakeTempFolder();
        }

        private void DoResponseMakeTempFolder(bool success, string createdTempFolderPath)
        {
            // レスポンスをチェック
            if (success == false) {
                NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_CREATE_TEMPDIR_FAIL);
                NotifyProcessTerminated(false);
                return;
            }

            // 生成された作業用フォルダー名称を保持
            TempFolderPath = createdTempFolderPath;
            AppCommon.OutputLogDebug(string.Format(ToolGUICommon.MSG_FORMAT_OPENPGP_CREATED_TEMPDIR, TempFolderPath));

            // コマンドに応じ、以下の処理に分岐
            switch (RequestType) {
                case AppCommon.RequestType.OpenPGPInstallKeys:
                    DoRequestGenerateMainKey();
                    break;
                case AppCommon.RequestType.OpenPGPReset:
                    DoRequestCardReset();
                    break;
                default:
                    NotifyProcessTerminated(false);
                    break;
            }
        }

        private void DoRequestGenerateMainKey()
        {
            // TODO: 仮の実装です。
            DoRequestRemoveTempFolder();
        }

        private void DoRequestCardReset()
        {
        }

        private void DoRequestRemoveTempFolder()
        {
            // 作業用フォルダーをPC上から削除
            RemoveTempFolder();
        }

        private void DoResponseRemoveTempFolder(bool success)
        {
            // レスポンスをチェック
            if (success == false) {
                NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_REMOVE_TEMPDIR_FAIL);
                NotifyProcessTerminated(false);
                return;
            }

            // 生成された作業用フォルダー名称をクリア
            TempFolderPath = null;
            AppCommon.OutputLogDebug(ToolGUICommon.MSG_OPENPGP_REMOVED_TEMPDIR);

            // 処理完了を通知
            NotifyProcessTerminated(true);
        }

        //
        // 内部処理
        //
        private bool CheckIfCardErrorFromResponse(string response)
        {
            // メッセージ検索用文字列
            string keyword = "selecting card failed";

            // 改行文字で区切られた文字列を分割
            string[] responseArray = Regex.Split(response, "\r\n|\n");
            foreach (string text in responseArray) {
                if (text.Contains(keyword)) {
                    return true;
                }
            }
            return false;
        }

        private void MakeTempFolder()
        {
            bool success = false;
            string tempFilePath = "";

            try {
                // 作業用フォルダーを生成
                tempFilePath = Path.GetTempFileName();
                File.Delete(tempFilePath);
                Directory.CreateDirectory(tempFilePath);
                success = true;

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("ToolPGP.MakeTempFolder exception:\n{0}", e.Message));
            }

            // 生成された作業用フォルダーを戻す
            DoResponseMakeTempFolder(success, tempFilePath);
        }

        private void RemoveTempFolder()
        {
            bool success = false;

            try {
                // 作業用フォルダーを、内包しているファイルごと削除
                Directory.Delete(TempFolderPath, true);
                success = true;

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("ToolPGP.RemoveTempFolder exception:\n{0}", e.Message));
            }

            // 作業用フォルダー削除の成否を戻す
            DoResponseRemoveTempFolder(success);
        }

        private bool CheckIfGPGVersionAvailable(string response)
        {
            // メッセージ検索用文字列
            string keyword = "gpg (GnuPG) ";

            // 改行文字で区切られた文字列を分割
            string[] responseArray = Regex.Split(response, "\r\n|\n");
            foreach (string text in responseArray) {
                if (text.StartsWith(keyword)) {
                    // バージョン文字列を抽出
                    string versionStr = text.Replace(keyword, "");
                    int versionDec = AppCommon.CalculateDecimalVersion(versionStr);

                    // PCに導入されているGnuPGのバージョンが2.3.4以上の場合は true
                    AppCommon.OutputLogDebug(string.Format("Installed GnuPG: version {0}", versionStr));
                    return (versionDec >= 20304);
                }
            }
            AppCommon.OutputLogDebug("GnuPG is not installed yet");
            return false;
        }

        //
        // GPGコマンドラインプロセッサー
        //
        private void DoRequestCommandLine(GPGCommand command, string commandPath, string commandArgs, string workingDirectory)
        {
            // 実行コマンドに関する諸設定
            ProcessStartInfo psi = new ProcessStartInfo();
            psi.FileName = commandPath;
            psi.Arguments = commandArgs;
            psi.UseShellExecute = false;
            psi.RedirectStandardOutput = true;
            psi.RedirectStandardError = true;
            psi.StandardOutputEncoding = Encoding.UTF8;
            psi.StandardErrorEncoding = Encoding.UTF8;
            if (workingDirectory != null) {
                psi.WorkingDirectory = workingDirectory;
            }

            // 出力格納領域を初期化
            string stdOutputString = "";
            string stdErrorString = "";
            bool success = false;

            try {
                // コマンドを実行
                Process child = Process.Start(psi);
                stdOutputString = child.StandardOutput.ReadToEnd();
                stdErrorString = child.StandardError.ReadToEnd();
                
                // コマンドからの応答を待機
                child.WaitForExit();

                // エラー出力があればログに出力
                if (stdErrorString != null && stdErrorString.Length > 0) {
                    AppCommon.OutputLogError(string.Format("ToolPGP.DoRequestCommandLine error:\n{0}", stdErrorString));
                }

                // コマンドの戻り値が０であれば true
                if (child.ExitCode == 0) {
                    success = true;
                }

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("ToolPGP.DoRequestCommandLine exception:\n{0}", e.Message));
            }

            // コマンドからの応答文字列を戻す
            if (success) {
                OnCommandTerminated(command, success, stdOutputString);
            } else {
                OnCommandTerminated(command, success, stdErrorString);
            }
        }

        private void OnCommandTerminated(GPGCommand command, bool success, string response)
        {
            // レスポンスを処理
            switch (command) {
                case GPGCommand.COMMAND_GPG_VERSION:
                    DoResponseGPGVersion(success, response);
                    break;
                case GPGCommand.COMMAND_GPG_CARD_STATUS:
                    DoResponseCardStatus(success, response);
                    break;
                default:
                    break;
            }
        }

        // 
        // 共通処理
        //
        private void NotifyProcessStarted(AppCommon.RequestType requestType)
        {
            // 処理機能に応じ、以下の処理に分岐
            RequestType = requestType;
            switch (RequestType) {
                case AppCommon.RequestType.OpenPGPInstallKeys:
                    NameOfCommand = ToolGUICommon.PROCESS_NAME_OPENPGP_INSTALL_KEYS;
                    break;
                case AppCommon.RequestType.OpenPGPStatus:
                    NameOfCommand = ToolGUICommon.PROCESS_NAME_OPENPGP_STATUS;
                    break;
                case AppCommon.RequestType.OpenPGPReset:
                    NameOfCommand = ToolGUICommon.PROCESS_NAME_OPENPGP_RESET;
                    break;
                case AppCommon.RequestType.HidFirmwareReset:
                    NameOfCommand = ToolGUICommon.PROCESS_NAME_FIRMWARE_RESET;
                    break;
                default:
                    NameOfCommand = "";
                    break;
            }

            // コマンド開始メッセージをログファイルに出力
            string startMsg = string.Format(ToolGUICommon.MSG_FORMAT_START_MESSAGE, NameOfCommand);
            AppCommon.OutputLogInfo(startMsg);
        }

        private void NotifyErrorMessage(string message)
        {
            // エラーメッセージをログファイルに出力（出力前に改行文字を削除）
            AppCommon.OutputLogError(message.Replace("\n", ""));

            // 戻り先画面に表示させるためのエラーメッセージを保持
            ErrorMessageOfCommand = message;
        }

        private void NotifyProcessTerminated(bool success)
        {
            // コマンドの実行結果をログ出力
            string formatted = string.Format(ToolGUICommon.MSG_FORMAT_END_MESSAGE,
                NameOfCommand,
                success ? ToolGUICommon.MSG_SUCCESS : ToolGUICommon.MSG_FAILURE);
            if (success) {
                AppCommon.OutputLogInfo(formatted);
            } else {
                AppCommon.OutputLogError(formatted);
            }

            // TODO: 画面に制御を戻す
            MessageBox.Show(mainForm, formatted, MainForm.MaintenanceToolTitle, MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
    }
}
