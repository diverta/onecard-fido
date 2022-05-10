using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    public class ToolPGPConst
    {
        public const byte OPENPGP_INS_SELECT = 0xA4;
        public const byte OPENPGP_INS_VERIFY = 0x20;
    }

    public class ToolPGPParameter
    {
        // 鍵作成用パラメーター
        public string RealName { get; set; }
        public string MailAddress { get; set; }
        public string Comment { get; set; }
        public string Passphrase { get; set; }
        public string PubkeyFolderPath { get; set; }
        public string BackupFolderPath { get; set; }
        public string CurrentPin { get; set; }
        public string NewPin { get; set; }
        public string NewPinForConfirm { get; set; }
        public AppCommon.RequestType SelectedPinCommand { get; set; }
        public string SelectedPinCommandName { get; set; }
    }

    public class ToolPGP
    {
        // OpenPGP機能設定画面
        private PGPPreferenceForm PreferenceForm;

        // メイン画面の参照を保持
        private MainForm MainFormRef;

        // 処理クラスの参照を保持
        private HIDMain HidMainRef;
        private ToolPGPCcid PGPCcid;

        // 処理機能を保持
        private AppCommon.RequestType RequestType;

        // 処理機能名を保持
        private string NameOfCommand;

        // エラーメッセージテキストを保持
        private string ErrorMessageOfCommand;

        // コマンドが成功したかどうかを保持
        private bool CommandSuccess;

        // ステータス照会情報を保持
        private string StatusInfoString;

        // 生成された作業用フォルダー名称を保持
        private string TempFolderPath;

        // 生成された鍵のIDを保持
        private string GeneratedMainKeyId;

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
            COMMAND_GPG_CARD_EDIT_PASSWD,
            COMMAND_GPG_CARD_EDIT_UNBLOCK,
        };

        // リクエストパラメーターを保持
        private ToolPGPParameter Parameter = null;

        public ToolPGP(MainForm f, HIDMain h)
        {
            // メイン画面の参照を保持
            MainFormRef = f;

            // HID処理クラスの参照を保持
            HidMainRef = h;

            // OpenPGP機能設定画面を生成
            PreferenceForm = new PGPPreferenceForm(this);

            // CCID処理クラスを生成
            PGPCcid = new ToolPGPCcid();
            PGPCcid.OnCcidCommandTerminated += OnCcidCommandTerminated;
            PGPCcid.OnCcidCommandNotifyErrorMessage += OnCcidCommandNotifyErrorMessage;
        }

        public void ShowDialog()
        {
            // ツール設定画面を表示
            PreferenceForm.ShowDialog();
        }

        public string GetPGPStatusInfoString()
        {
            return StatusInfoString;
        }

        public bool CheckUSBDeviceDisconnected()
        {
            return MainFormRef.CheckUSBDeviceDisconnected();
        }

        //
        // ファームウェアリセット用関数
        //
        public void DoCommandResetFirmware()
        {
            // HIDインターフェース経由でファームウェアをリセット
            AppCommon.RequestType requestType = AppCommon.RequestType.HidFirmwareReset;
            NotifyProcessStarted(requestType);
            HidMainRef.DoFirmwareReset(requestType, this);
        }

        public void DoResponseResetFirmware(bool success)
        {
            if (success == false) {
                NotifyErrorMessage(ToolGUICommon.MSG_FIRMWARE_RESET_UNSUPP);
            }
            NotifyProcessTerminated(success);
        }

        //
        // OpenPGP機能設定用関数
        // 
        public void DoOpenPGPCommand(AppCommon.RequestType requestType, ToolPGPParameter parameter)
        {
            // 画面から引き渡されたパラメーターを退避
            Parameter = parameter;

            // コマンド開始処理
            NotifyProcessStarted(requestType);

            // コマンドを別スレッドで起動
            Task task = Task.Run(() => {
                if (requestType == AppCommon.RequestType.OpenPGPInstallKeys) {
                    // 管理用PIN番号検証から開始
                    DoRequestAdminPinVerify();

                } else {
                    // バージョン照会から開始
                    DoRequestGPGVersion();
                }
            });

            // 進捗画面を表示
            CommonProcessingForm.OpenForm(PreferenceForm);
        }

        //
        // CCID I/Fコマンド実行関数
        //
        private void DoRequestAdminPinVerify()
        {
            // 事前にCCID I/F経由で、管理用PIN番号の検証を試行
            PGPCcid.DoOpenPGPCcidCommand(RequestType, Parameter);
        }

        private void DoResponseAdminPinVerify(bool success)
        {
            if (success) {
                // バージョン照会から開始
                AppUtil.OutputLogDebug(ToolGUICommon.MSG_OPENPGP_ADMIN_PIN_VERIFIED);
                DoRequestGPGVersion();

            } else {
                // 画面に制御を戻す
                NotifyProcessTerminated(false);
            }
        }

        //
        // GPGコマンド実行関数
        // 
        private void DoRequestGPGVersion()
        {
            // インストールされているGPGコマンドのバージョンを照会
            DoRequestCommandLine(GPGCommand.COMMAND_GPG_VERSION, "gpg", "--version", null);
        }

        private void DoResponseGPGVersion(bool success, string response, string error)
        {
            // PCに導入されているGPGが、所定のバージョン以上でない場合は終了
            if (success == false || CheckIfGPGVersionAvailable(response) == false) {
                NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_GPG_VERSION_UNAVAIL);
                NotifyProcessTerminated(false);
                return;
            }

            // 次の処理に移行
            DoRequestMakeTempFolder();
        }

        private void DoRequestCardStatus()
        {
            // スクリプトを作業用フォルダーに生成
            string scriptName = "card_status.bat";
            if (WriteScriptToTempFolder(scriptName) == false) {
                NotifyProcessTerminated(false);
                return;
            }

            // スクリプトを実行
            string exe = string.Format("{0}\\{1}", TempFolderPath, scriptName);
            DoRequestCommandLine(GPGCommand.COMMAND_GPG_CARD_STATUS, exe, "", TempFolderPath);
        }

        private void DoResponseCardStatus(bool success, string response, string error)
        {
            // レスポンスをチェック
            if (success) {
                // レスポンスを保持
                StatusInfoString = response;
                CommandSuccess = true;

            } else {
                // スクリプトエラーの場合はOpenPGP cardエラーをチェック
                if (CheckIfCardErrorFromResponse(error)) {
                    NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_SELECTING_CARD_FAIL);
                } else {
                    NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_STATUS_COMMAND_FAIL);
                }
            }

            // 後処理に移行
            DoRequestRemoveTempFolder();
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
            AppUtil.OutputLogDebug(string.Format(ToolGUICommon.MSG_FORMAT_OPENPGP_CREATED_TEMPDIR, TempFolderPath));

            // コマンドに応じ、以下の処理に分岐
            switch (RequestType) {
                case AppCommon.RequestType.OpenPGPStatus:
                    DoRequestCardStatus();
                    break;
                case AppCommon.RequestType.OpenPGPInstallKeys:
                    DoRequestGenerateMainKey();
                    break;
                case AppCommon.RequestType.OpenPGPReset:
                    DoRequestCardReset();
                    break;
                case AppCommon.RequestType.OpenPGPChangePin:
                case AppCommon.RequestType.OpenPGPChangeAdminPin:
                case AppCommon.RequestType.OpenPGPUnblockPin:
                case AppCommon.RequestType.OpenPGPSetResetCode:
                    DoRequestCardEditPasswd();
                    break;
                case AppCommon.RequestType.OpenPGPUnblock:
                    DoRequestCardEditUnblock();
                    break;
                default:
                    NotifyProcessTerminated(false);
                    break;
            }
        }

        private void DoRequestGenerateMainKey()
        {
            // スクリプトを作業用フォルダーに生成
            string scriptName = "generate_main_key.bat";
            if (WriteScriptToTempFolder(scriptName) == false) {
                NotifyProcessTerminated(false);
                return;
            }

            // パラメーターファイルを作業用フォルダーに生成
            string paramName = "generate_main_key.param";
            if (WriteParamForGenerateMainKeyToTempFolder(paramName) == false) {
                NotifyProcessTerminated(false);
                return;
            }

            // スクリプトを実行
            string exe = string.Format("{0}\\{1}", TempFolderPath, scriptName);
            string param = string.Format("{0} {1} --no-tty", TempFolderPath, Parameter.Passphrase);
            DoRequestCommandLine(GPGCommand.COMMAND_GPG_GENERATE_MAIN_KEY, exe, param, TempFolderPath);
        }

        private void DoResponseGenerateMainKey(bool success, string response, string error)
        {
            // 生成鍵IDをクリア
            GeneratedMainKeyId = null;

            if (CheckResponseOfScript(response)) {
                // 生成鍵がCertify機能を有しているかチェック
                string keyid = ExtractMainKeyIdFromResponse(response);
                if (keyid != null) {
                    // チェックOKの場合は鍵IDを保持し、次の処理に移行
                    GeneratedMainKeyId = keyid;
                    AppUtil.OutputLogDebug(string.Format(ToolGUICommon.MSG_FORMAT_OPENPGP_GENERATED_MAIN_KEY, GeneratedMainKeyId));
                    DoRequestAddSubKey();
                    return;
                }
            }

            // エラーメッセージを出力し、後処理に移行
            NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_GENERATE_MAINKEY_FAIL);
            DoRequestRemoveTempFolder();
        }

        private void DoRequestAddSubKey()
        {
            // スクリプトを作業用フォルダーに生成
            string scriptName = "add_sub_key.bat";
            if (WriteScriptToTempFolder(scriptName) == false) {
                NotifyProcessTerminated(false);
                return;
            }

            // パラメーターファイルを作業用フォルダーに生成
            string paramName = "add_sub_key.param";
            if (WriteScriptToTempFolder(paramName) == false) {
                NotifyProcessTerminated(false);
                return;
            }

            // スクリプトを実行
            string exe = string.Format("{0}\\{1}", TempFolderPath, scriptName);
            string param = string.Format("{0} {1} {2} --no-tty", TempFolderPath, Parameter.Passphrase, GeneratedMainKeyId);
            DoRequestCommandLine(GPGCommand.COMMAND_GPG_ADD_SUB_KEY, exe, param, TempFolderPath);
        }

        private void DoResponseAddSubKey(bool success, string response, string error)
        {
            // レスポンスをチェック
            if (CheckResponseOfScript(response)) {
                if (CheckIfSubKeysExistFromResponse(response, false)) {
                    // 副鍵が３点生成された場合は、次の処理に移行
                    AppUtil.OutputLogDebug(ToolGUICommon.MSG_OPENPGP_ADDED_SUB_KEYS);
                    DoRequestExportPubkeyAndBackup();
                    return;
                }
            }

            // エラーメッセージを出力し、後処理に移行
            NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_GENERATE_SUB_KEY_FAIL);
            DoRequestRemoveTempFolder();
        }

        private void DoRequestExportPubkeyAndBackup()
        {
            // スクリプトを作業用フォルダーに生成
            string scriptName = "export_pubkey_and_backup.bat";
            if (WriteScriptToTempFolder(scriptName) == false) {
                NotifyProcessTerminated(false);
                return;
            }

            // スクリプトを実行
            string exe = string.Format("{0}\\{1}", TempFolderPath, scriptName);
            string param = string.Format("{0} {1} {2} {3} {4}", TempFolderPath, Parameter.Passphrase, GeneratedMainKeyId,
                Parameter.PubkeyFolderPath, Parameter.BackupFolderPath);
            DoRequestCommandLine(GPGCommand.COMMAND_GPG_EXPORT_PUBKEY_AND_BACKUP, exe, param, TempFolderPath);
        }

        private void DoResponseExportPubkeyAndBackup(bool success, string response, string error)
        {
            // レスポンスをチェック
            if (CheckResponseOfScript(response)) {
                if (CheckIfPubkeyAndBackupExist()) {
                    // 公開鍵ファイル、バックアップファイルが生成された場合は、次の処理に移行
                    AppUtil.OutputLogDebug(string.Format(ToolGUICommon.MSG_FORMAT_OPENPGP_EXPORT_PUBKEY_DONE, Parameter.PubkeyFolderPath));
                    AppUtil.OutputLogDebug(string.Format(ToolGUICommon.MSG_FORMAT_OPENPGP_EXPORT_BACKUP_DONE, Parameter.BackupFolderPath));
                    DoRequestTransferSubkeyToCard();
                    return;
                }
            }

            // エラーメッセージを出力し、後処理に移行
            NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_EXPORT_BACKUP_FAIL);
            DoRequestRemoveTempFolder();
        }

        private void DoRequestTransferSubkeyToCard()
        {
            // スクリプトを作業用フォルダーに生成
            string scriptName = "transfer_subkey_to_card.bat";
            if (WriteScriptToTempFolder(scriptName) == false) {
                NotifyProcessTerminated(false);
                return;
            }

            // パラメーターファイルを作業用フォルダーに生成
            string paramName = "transfer_subkey_to_card.param";
            if (WriteScriptToTempFolder(paramName) == false) {
                NotifyProcessTerminated(false);
                return;
            }

            // スクリプトを実行
            string exe = string.Format("{0}\\{1}", TempFolderPath, scriptName);
            string param = string.Format("{0} {1} {2} --no-tty", TempFolderPath, Parameter.Passphrase, GeneratedMainKeyId);
            DoRequestCommandLine(GPGCommand.COMMAND_GPG_TRANSFER_SUBKEY_TO_CARD, exe, param, TempFolderPath);
        }

        private void DoResponseTransferSubkeyToCard(bool success, string response, string error)
        {
            // レスポンスをチェック
            if (CheckResponseOfScript(response)) {
                if (CheckIfSubKeysExistFromResponse(response, true)) {
                    // 副鍵が認証器に移動された場合は、処理成功を通知
                    AppUtil.OutputLogDebug(ToolGUICommon.MSG_OPENPGP_TRANSFERRED_KEYS_TO_DEVICE);
                    CommandSuccess = true;

                } else {
                    // 副鍵が移動されなかった場合、副鍵が認証器に既に保管されていたかどうかチェック
                    // （標準エラーに出力されるメッセージをチェック）
                    if (CheckIfSubKeyAlreadyStoredFromResponse(error)) {
                        NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_KEYS_ALREADY_STORED);
                    } else {
                        NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_TRANSFER_KEYS_FAIL);
                    }
                }

            } else {
                // スクリプトエラーの場合はOpenPGP cardエラーをチェック
                if (CheckIfCardErrorFromResponse(error)) {
                    NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_SELECTING_CARD_FAIL);
                } else {
                    NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_TRANSFER_SCRIPT_FAIL);
                }
            }

            // 後処理に移行
            DoRequestRemoveTempFolder();
        }

        private void DoRequestCardReset()
        {
            // スクリプトを作業用フォルダーに生成
            string scriptName = "card_reset.bat";
            if (WriteScriptToTempFolder(scriptName) == false) {
                NotifyProcessTerminated(false);
                return;
            }

            // パラメーターファイルを作業用フォルダーに生成
            string paramName = "card_reset.param";
            if (WriteScriptToTempFolder(paramName) == false) {
                NotifyProcessTerminated(false);
                return;
            }

            // スクリプトを実行
            string exe = string.Format("{0}\\{1}", TempFolderPath, scriptName);
            string param = string.Format("{0} --no-tty", TempFolderPath);
            DoRequestCommandLine(GPGCommand.COMMAND_GPG_CARD_RESET, exe, param, TempFolderPath);
        }

        private void DoResponseCardReset(bool success, string response, string error)
        {
            // レスポンスをチェック
            if (success == false) {
                // スクリプトエラーの場合はOpenPGP cardエラーをチェック
                if (CheckIfCardErrorFromResponse(error)) {
                    NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_SELECTING_CARD_FAIL);
                } else {
                    NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_SUBKEY_REMOVE_FAIL);
                }

            } else {
                // スクリプト正常終了の場合は副鍵３点が存在しない事をチェック
                if (CheckIfNoSubKeyExistFromResponse(response)) {
                    CommandSuccess = true;
                } else {
                    NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_SUBKEY_NOT_REMOVED);
                }
            }

            // 後処理に移行
            DoRequestRemoveTempFolder();
        }

        private void DoRequestCardEditPasswd()
        {
            DoRequestCardEditPasswdCommand(GPGCommand.COMMAND_GPG_CARD_EDIT_PASSWD);
        }

        private void DoRequestCardEditUnblock()
        {
            DoRequestCardEditPasswdCommand(GPGCommand.COMMAND_GPG_CARD_EDIT_UNBLOCK);
        }

        private void DoRequestCardEditPasswdCommand(GPGCommand command)
        {
            // スクリプトを作業用フォルダーに生成
            string scriptName = "card_edit_passwd.bat";
            if (WriteScriptToTempFolder(scriptName) == false) {
                NotifyProcessTerminated(false);
                return;
            }

            // パラメーターファイル名を設定
            string paramName;
            switch (command) {
                case GPGCommand.COMMAND_GPG_CARD_EDIT_UNBLOCK:
                    paramName = "card_edit_unblock.param";
                    break;
                default:
                    paramName = "card_edit_passwd.param";
                    break;
            }

            // パラメーターファイルを作業用フォルダーに生成
            if (WriteParamForCardEditUnblockToTempFolder(paramName, command) == false) {
                NotifyProcessTerminated(false);
                return;
            }

            // スクリプトを実行
            string exe = string.Format("{0}\\{1}", TempFolderPath, scriptName);
            string param = string.Format("{0} {1} --no-tty", TempFolderPath, paramName);
            DoRequestCommandLine(command, exe, param, TempFolderPath);
        }

        private void DoResponseCardEditPasswdCommand(bool success, string response, string error)
        {
            // レスポンスをチェック
            if (success == false) {
                // スクリプトエラーの場合はOpenPGP cardエラーをチェック
                if (CheckIfCardErrorFromResponse(error)) {
                    NotifyErrorMessage(ToolGUICommon.MSG_ERROR_OPENPGP_SELECTING_CARD_FAIL);
                } else {
                    string message = string.Format(ToolGUICommon.MSG_FORMAT_OPENPGP_CARD_EDIT_PASSWD_ERR, Parameter.SelectedPinCommandName);
                    NotifyErrorMessage(message);
                }

            } else {
                // 成功 or 失敗メッセージが出力されているかどうかチェック
                if (CheckIfOperationSuccess(response)) {
                    CommandSuccess = true;
                } else {
                    string itemName = ItemNameForCardEditPasswdCommand();
                    string message = string.Format(ToolGUICommon.MSG_FORMAT_OPENPGP_CARD_EDIT_PASSWD_NG, itemName);
                    NotifyErrorMessage(message);
                }
            }

            // 後処理に移行
            DoRequestRemoveTempFolder();
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
            AppUtil.OutputLogDebug(ToolGUICommon.MSG_OPENPGP_REMOVED_TEMPDIR);

            // 処理完了を通知
            NotifyProcessTerminated(CommandSuccess);
        }

        //
        // 内部処理
        //
        private bool CheckResponseOfScript(string response)
        {
            // メッセージ検索用文字列
            string keyword = "Execute script for gnupg success";

            // 改行文字で区切られた文字列を分割
            foreach (string text in TextArrayOfResponse(response)) {
                if (text.Contains(keyword)) {
                    // シェルスクリプトから成功メッセージが出力された場合、trueを戻す
                    return true;
                }
            }
            return false;
        }

        private bool CheckIfCardErrorFromResponse(string response)
        {
            // メッセージ検索用文字列
            string keyword = "selecting card failed";

            // 改行文字で区切られた文字列を分割
            foreach (string text in TextArrayOfResponse(response)) {
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
                AppUtil.OutputLogError(string.Format("ToolPGP.MakeTempFolder exception:\n{0}", e.Message));
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
                AppUtil.OutputLogError(string.Format("ToolPGP.RemoveTempFolder exception:\n{0}", e.Message));
            }

            // 作業用フォルダー削除の成否を戻す
            DoResponseRemoveTempFolder(success);
        }

        private bool CheckIfGPGVersionAvailable(string response)
        {
            // メッセージ検索用文字列
            string keyword = "gpg (GnuPG) ";

            // 改行文字で区切られた文字列を分割
            foreach (string text in TextArrayOfResponse(response)) {
                if (text.StartsWith(keyword)) {
                    // バージョン文字列を抽出
                    string versionStr = text.Replace(keyword, "");
                    int versionDec = AppUtil.CalculateDecimalVersion(versionStr);

                    // PCに導入されているGnuPGのバージョンが2.3.4以上の場合は true
                    AppUtil.OutputLogDebug(string.Format("Installed GnuPG: version {0}", versionStr));
                    return (versionDec >= 20304);
                }
            }
            AppUtil.OutputLogDebug("GnuPG is not installed yet");
            return false;
        }

        string ExtractMainKeyIdFromResponse(string response)
        {
            // メッセージ文字列から鍵IDを抽出
            string keyid = null;

            // メッセージ検索用文字列
            string keyword = "pub   rsa2048";

            // 改行文字で区切られた文字列を分割
            string[] textArray = TextArrayOfResponse(response);

            // 分割されたメッセージの１件目について、鍵の機能を解析
            if (textArray[0].StartsWith(keyword)) {
                if (textArray[0].Contains("[C]")) {
                    // 分割されたメッセージの２件目、後ろから16バイトの文字列を、鍵IDとして抽出
                    int startIndex = textArray[1].Length - 16;
                    keyid = textArray[1].Substring(startIndex);
                }
            }

            // 抽出された鍵IDを戻す
            return keyid;
        }

        private bool CheckIfSubKeysExistFromResponse(string response, bool transferred)
        {
            // メッセージ検索用文字列
            string keyword1 = string.Format("{0}\\pubring.kbx", TempFolderPath);
            string keyword2 = transferred ? "ssb>  rsa2048" : "ssb   rsa2048";

            // 副鍵生成の有無を保持
            bool subKeyS = false;
            bool subKeyE = false;
            bool subKeyA = false;

            // メッセージ文字列から鍵一覧メッセージ（'gpg -K'実行結果）を抽出
            bool gpgKisAvailable = false;
            foreach (string text in TextArrayOfResponse(response)) {
                if (text.StartsWith(keyword1)) {
                    // 'gpg -K'の実行結果が、メッセージ文字列中に存在すると判断
                    gpgKisAvailable = true;
                    continue;
                }
                // 'gpg -K'の実行結果を解析
                if (gpgKisAvailable) {
                    // 副鍵に関するメッセージを解析
                    if (text.StartsWith(keyword2)) {
                        // 副鍵の機能を解析
                        if (text.Contains("[S]")) {
                            subKeyS = true;
                        } else if (text.Contains("[E]")) {
                            subKeyE = true;
                        } else if (text.Contains("[A]")) {
                            subKeyA = true;
                        }
                    }
                }
            }

            // ３点の副鍵が揃っていれば true を戻す
            if (subKeyS && subKeyE && subKeyA) {
                return true;
            }

            // 揃っていない副鍵についてログを出力
            string str = transferred ? "transferred" : "added";
            if (subKeyS == false) {
                AppUtil.OutputLogDebug(string.Format("Sub key (for sign) not {0}", str));
            }
            if (subKeyE == false) {
                AppUtil.OutputLogDebug(string.Format("Sub key (for encrypt) not {0}", str));
            }
            if (subKeyA == false) {
                AppUtil.OutputLogDebug(string.Format("Sub key (for authenticate) not {0}", str));
            }

            // false を戻す
            return false;
        }

        private bool CheckIfPubkeyAndBackupExist()
        {
            // 公開鍵ファイルがエクスポート先に存在するかチェック
            if (CheckIfFileExist("public_key.pgp", Parameter.PubkeyFolderPath) == false) {
                AppUtil.OutputLogError(ToolGUICommon.MSG_ERROR_OPENPGP_EXPORT_PUBKEY_FAIL);
                return false;
            }

            // バックアップファイルがエクスポート先に存在するかチェック
            if (CheckIfFileExist("GNUPGHOME.tgz", Parameter.BackupFolderPath) == false) {
                AppUtil.OutputLogError(ToolGUICommon.MSG_ERROR_OPENPGP_BACKUP_FAIL);
                return false;
            }

            return true;
        }

        private bool CheckIfFileExist(string filename, string inFolderPath)
        {
            // 指定のフォルダー配下にファイルが存在している場合は true
            string filePath = string.Format("{0}\\{1}", inFolderPath, filename);
            return File.Exists(filePath);
        }

        private bool CheckIfSubKeyAlreadyStoredFromResponse(string response)
        {
            // メッセージ検索用文字列
            string keyword = "such a key has already been stored on the card!";

            // 改行文字で区切られた文字列を分割
            foreach (string text in TextArrayOfResponse(response)) {
                if (text.Contains(keyword)) {
                    return true;
                }
            }
            return false;
        }

        private bool CheckIfNoSubKeyExistFromResponse(string response)
        {
            // ステータス開始行の有無を保持
            bool header = false;

            // 副鍵の有無を保持
            bool subKeyS = false;
            bool subKeyE = false;
            bool subKeyA = false;

            // 改行文字で区切られた文字列を分割
            foreach (string text in TextArrayOfResponse(response)) {
                if (text.Contains("Reader ...........:")) {
                    header = true;
                } else if (text.Contains("Signature key")) {
                    subKeyS = text.Contains("[none]");
                } else if (text.Contains("Encryption key")) {
                    subKeyE = text.Contains("[none]");
                } else if (text.Contains("Authentication key")) {
                    subKeyA = text.Contains("[none]");
                }
            }

            // ３点の副鍵が削除されていれば true を戻す
            return (header && subKeyS && subKeyE && subKeyA);
        }

        private bool CheckIfOperationSuccess(string response)
        {
            // メッセージ検索用文字列
            string keywordNG = "SC_OP_FAILURE";
            string keywordOK = "SC_OP_SUCCESS";

            // 改行文字で区切られた文字列を分割
            foreach (string text in TextArrayOfResponse(response)) {
                if (text.Contains(keywordNG)) {
                    // 失敗メッセージが出力されている場合は false
                    AppUtil.OutputLogError(string.Format("GnuPG operation failed: {0}", text));
                    return false;

                } else if (text.Contains(keywordOK)) {
                    // 成功メッセージが出力されている場合は true
                    return true;
                }
            }

            // 所定のメッセージが出力されていない場合は false
            return false;
        }

        //
        // スクリプト／パラメーターファイル関連
        //
        private bool WriteScriptToTempFolder(string scriptName)
        {
            // スクリプトをリソースから読込み
            string scriptContent = GetScriptResourceContentString(scriptName);
            if (scriptContent == null) {
                return false;
            }

            // スクリプトファイルを作業用フォルダーに書き出し
            string scriptFilePath = string.Format("{0}\\{1}", TempFolderPath, scriptName);
            if (WriteStringToFile(scriptContent, scriptFilePath) == false) {
                return false;
            }

            return true;
        }

        private bool WriteParamForGenerateMainKeyToTempFolder(string scriptName)
        {
            // パラメーターをリソースから読込み
            string scriptContent = GetScriptResourceContentString(scriptName);
            if (scriptContent == null) {
                return false;
            }

            // パラメーターを置き換え
            string parameterContent = string.Format(scriptContent, Parameter.RealName, Parameter.MailAddress, Parameter.Comment);

            // パラメーターファイルを作業用フォルダーに書き出し
            string scriptFilePath = string.Format("{0}\\{1}", TempFolderPath, scriptName);
            if (WriteStringToFile(parameterContent, scriptFilePath) == false) {
                return false;
            }

            return true;
        }

        private bool WriteParamForCardEditUnblockToTempFolder(string scriptName, GPGCommand command)
        {
            // パラメーターをリソースから読込み
            string scriptContent = GetScriptResourceContentString(scriptName);
            if (scriptContent == null) {
                return false;
            }

            // パラメーターを置き換え
            string parameterContent;
            switch (command) {
                case GPGCommand.COMMAND_GPG_CARD_EDIT_UNBLOCK:
                    parameterContent = string.Format(scriptContent, Parameter.CurrentPin, Parameter.NewPin, Parameter.NewPinForConfirm);
                    break;
                default:
                    parameterContent = string.Format(scriptContent, MenuNoForCardEditPasswdCommand(), Parameter.CurrentPin, Parameter.NewPin, Parameter.NewPinForConfirm);
                    break;
            }

            // パラメーターファイルを作業用フォルダーに書き出し
            string scriptFilePath = string.Format("{0}\\{1}", TempFolderPath, scriptName);
            if (WriteStringToFile(parameterContent, scriptFilePath) == false) {
                return false;
            }

            return true;
        }

        private string MenuNoForCardEditPasswdCommand()
        {
            switch (Parameter.SelectedPinCommand) {
                case AppCommon.RequestType.OpenPGPChangePin:
                    return "1";
                case AppCommon.RequestType.OpenPGPUnblockPin:
                    return "2";
                case AppCommon.RequestType.OpenPGPChangeAdminPin:
                    return "3";
                case AppCommon.RequestType.OpenPGPSetResetCode:
                    return "4";
                default:
                    return "Q";
            }
        }

        private string ItemNameForCardEditPasswdCommand()
        {
            switch (Parameter.SelectedPinCommand) {
            case AppCommon.RequestType.OpenPGPUnblockPin:
            case AppCommon.RequestType.OpenPGPChangeAdminPin:
            case AppCommon.RequestType.OpenPGPSetResetCode:
                return ToolGUICommon.MSG_LABEL_ITEM_PGP_ADMIN_PIN;
            case AppCommon.RequestType.OpenPGPUnblock:
                return ToolGUICommon.MSG_LABEL_ITEM_PGP_RESET_CODE;
            default:
                return ToolGUICommon.MSG_LABEL_ITEM_PGP_PIN;
            }
        }

        private string GetScriptResourceContentString(string scriptName)
        {
            // スクリプトをリソースから読込み
            string scriptResourceName = GetScriptResourceName(scriptName);
            if (scriptResourceName == null) {
                AppUtil.OutputLogError(string.Format("Script resource name is null: {0}", scriptName));
                return null;
            }
            string scriptContent = GetScriptResourceContent(scriptResourceName);
            if (scriptContent == null) {
                AppUtil.OutputLogError(string.Format("Script content is null: {0}", scriptResourceName));
                return null;
            }
            return scriptContent;
        }

        private string GetScriptResourceName(string scriptName)
        {
            // 検索対象のリソース名
            string resourceName = string.Format("MaintenanceToolGUI.Resources.{0}", scriptName);

            // このアプリケーションに同梱されているリソース名を取得
            Assembly myAssembly = Assembly.GetExecutingAssembly();
            string[] resnames = myAssembly.GetManifestResourceNames();
            foreach (string resName in resnames) {
                // リソース名が
                // "MaintenanceToolGUI.Resources.<scriptName>"
                // という名称の場合
                if (resName.Equals(resourceName)) {
                    return resourceName;
                }
            }
            return null;
        }

        // スクリプト内容を読込むための領域
        private byte[] ScriptContentBytes = new byte[5120];
        private int ScriptContentSize { get; set; }

        private string GetScriptResourceContent(string resourceName)
        {
            // リソースファイルを開く
            Assembly assembly = Assembly.GetExecutingAssembly();
            Stream stream = assembly.GetManifestResourceStream(resourceName);
            if (stream == null) {
                return null;
            }

            try {
                // リソースファイルを配列に読込
                ScriptContentSize = stream.Read(ScriptContentBytes, 0, (int)stream.Length);

                // リソースファイルを閉じる
                stream.Close();

            } catch (Exception e) {
                AppUtil.OutputLogError(string.Format("ToolPGP.GetScriptResourceContent exception:\n{0}", e.Message));
                return null;
            }

            // 読込んだスクリプト内容を戻す
            byte[] b = ScriptContentBytes.Take(ScriptContentSize).ToArray();
            string text = Encoding.UTF8.GetString(b);
            return text;
        }

        bool WriteStringToFile(string contents, string filePath)
        {
            try {
                File.WriteAllText(filePath, contents);
                return true;

            } catch (Exception e) {
                AppUtil.OutputLogError(string.Format("ToolPGP.WriteStringToFile exception:\n{0}", e.Message));
                return false;
            }
        }

        string[] TextArrayOfResponse(string response)
        {
            return Regex.Split(response, "\r\n|\n");
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
            psi.CreateNoWindow = true;

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

                // コマンドの戻り値が０であれば true
                if (child.ExitCode == 0) {
                    success = true;
                }

            } catch (Exception e) {
                AppUtil.OutputLogError(string.Format("ToolPGP.DoRequestCommandLine exception:\n{0}", e.Message));
            }

            // コマンドからの応答文字列／エラー出力を戻す
            OnCommandTerminated(command, success, stdOutputString, stdErrorString);
        }

        private void OnCommandTerminated(GPGCommand command, bool success, string response, string error)
        {
            // レスポンスを処理
            switch (command) {
                case GPGCommand.COMMAND_GPG_VERSION:
                    DoResponseGPGVersion(success, response, error);
                    break;
                case GPGCommand.COMMAND_GPG_CARD_STATUS:
                    DoResponseCardStatus(success, response, error);
                    break;
                case GPGCommand.COMMAND_GPG_GENERATE_MAIN_KEY:
                    DoResponseGenerateMainKey(success, response, error);
                    break;
                case GPGCommand.COMMAND_GPG_ADD_SUB_KEY:
                    DoResponseAddSubKey(success, response, error);
                    break;
                case GPGCommand.COMMAND_GPG_EXPORT_PUBKEY_AND_BACKUP:
                    DoResponseExportPubkeyAndBackup(success, response, error);
                    break;
                case GPGCommand.COMMAND_GPG_TRANSFER_SUBKEY_TO_CARD:
                    DoResponseTransferSubkeyToCard(success, response, error);
                    break;
                case GPGCommand.COMMAND_GPG_CARD_RESET:
                    DoResponseCardReset(success, response, error);
                    break;
                case GPGCommand.COMMAND_GPG_CARD_EDIT_PASSWD:
                case GPGCommand.COMMAND_GPG_CARD_EDIT_UNBLOCK:
                    DoResponseCardEditPasswdCommand(success, response, error);
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
            // コマンド処理結果を初期化
            CommandSuccess = false;

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
                case AppCommon.RequestType.OpenPGPChangePin:
                case AppCommon.RequestType.OpenPGPChangeAdminPin:
                case AppCommon.RequestType.OpenPGPUnblockPin:
                case AppCommon.RequestType.OpenPGPSetResetCode:
                case AppCommon.RequestType.OpenPGPUnblock:
                    NameOfCommand = Parameter.SelectedPinCommandName;
                    break;
                default:
                    NameOfCommand = "";
                    break;
            }

            // コマンド開始メッセージをログファイルに出力
            string startMsg = string.Format(ToolGUICommon.MSG_FORMAT_START_MESSAGE, NameOfCommand);
            AppUtil.OutputLogInfo(startMsg);
        }

        private void NotifyErrorMessage(string message)
        {
            // エラーメッセージをログファイルに出力（出力前に改行文字を削除）
            AppUtil.OutputLogError(message.Replace("\n", ""));

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
                AppUtil.OutputLogInfo(formatted);
            } else {
                AppUtil.OutputLogError(formatted);
            }

            // 進捗画面を閉じる
            CommonProcessingForm.NotifyTerminate();

            // 画面に制御を戻す
            PreferenceForm.OnCommandProcessTerminated(RequestType, success, ErrorMessageOfCommand);
        }

        //
        // ToolPGPCcidクラスからのコールバック
        //
        private void OnCcidCommandNotifyErrorMessage(string errorMessage)
        {
            NotifyErrorMessage(errorMessage);
        }

        private void OnCcidCommandTerminated(bool success)
        {
            // コマンドに応じ、以下の処理に分岐
            switch (RequestType) {
                case AppCommon.RequestType.OpenPGPInstallKeys:
                    DoResponseAdminPinVerify(success);
                    break;

                default:
                    NotifyProcessTerminated(false);
                    break;
            }
        }
    }
}
