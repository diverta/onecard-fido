using System;
using System.IO;
using System.Windows.Forms;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    public partial class PGPPreferenceForm : Form
    {
        // 入力可能文字数
        private const int OPENPGP_NAME_SIZE_MIN = 5;
        private const int OPENPGP_ENTRY_SIZE_MAX = 32;
        private const int OPENPGP_ADMIN_PIN_CODE_SIZE_MIN = 8;
        private const int OPENPGP_ADMIN_PIN_CODE_SIZE_MAX = 8;

        // ASCII項目入力パターン [ -z]（表示可能な半角文字はすべて許容）
        private const string OPENPGP_ENTRY_PATTERN_ASCII = "^[ -z]+$";

        // ASCII項目入力パターン [ -z]（両端の半角スペースは許容しない）
        private const string OPENPGP_ENTRY_PATTERN_NOSP_BOTH_ENDS = "^[!-z]+[ -z]*[!-z]+$";

        // メールアドレス入力パターン \w は [a-zA-Z_0-9] と等価
        private const string OPENPGP_ENTRY_PATTERN_MAIL_ADDRESS = "^\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*$";

        // 処理クラスの参照を保持
        private ToolPGP ToolPGPRef;

        // 設定パラメーターを保持
        private ToolPGPParameter parameter = new ToolPGPParameter();

        public PGPPreferenceForm(ToolPGP toolPGP)
        {
            // 画面項目の初期化
            InitializeComponent();
            InitFieldValue();

            // 処理クラスの参照を保持
            ToolPGPRef = toolPGP;
        }

        private void buttonPubkeyFolderPath_Click(object sender, EventArgs e)
        {
            // フォルダーを選択
            FormUtil.SelectFolderPath(folderBrowserDialog1, AppCommon.MSG_PROMPT_SELECT_PGP_PUBKEY_FOLDER, textPubkeyFolderPath);
        }

        private void buttonBackupFolderPath_Click(object sender, EventArgs e)
        {
            // フォルダーを選択
            FormUtil.SelectFolderPath(folderBrowserDialog1, AppCommon.MSG_PROMPT_SELECT_PGP_BACKUP_FOLDER, textBackupFolderPath);
        }

        private void buttonInstallPGPKey_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (ToolPGPRef.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 入力欄の内容をチェック
            if (CheckForInstallPGPKey()) {
                // 画面入力内容を引数とし、PGP秘密鍵インストール処理を実行
                EnableButtons(false);
                DoCommandInstallPGPKey();
            }
        }

        private void buttonPGPStatus_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (ToolPGPRef.CheckUSBDeviceDisconnected()) {
                return;
            }

            // PGPステータス照会処理を実行
            EnableButtons(false);
            DoCommandPGPStatus();
        }

        private void buttonPGPReset_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (ToolPGPRef.CheckUSBDeviceDisconnected()) {
                return;
            }

            // プロンプトで表示されるタイトル
            string title = string.Format(
                AppCommon.MSG_FORMAT_OPENPGP_WILL_PROCESS,
                AppCommon.MSG_LABEL_COMMAND_OPENPGP_RESET);

            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (FormUtil.DisplayPromptPopup(this, title, AppCommon.MSG_PROMPT_OPENPGP_RESET) == false) {
                return;
            }

            // PGPリセット処理を実行
            EnableButtons(false);
            DoCommandPGPReset();
        }

        private void buttonFirmwareReset_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (ToolPGPRef.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 認証器のファームウェアを再起動
            EnableButtons(false);
            DoCommandResetFirmware();
        }

        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {
            GetSelectedPinCommandValue(sender);
        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {
            GetSelectedPinCommandValue(sender);
        }

        private void radioButton3_CheckedChanged(object sender, EventArgs e)
        {
            GetSelectedPinCommandValue(sender);
        }

        private void radioButton4_CheckedChanged(object sender, EventArgs e)
        {
            GetSelectedPinCommandValue(sender);
        }

        private void radioButton5_CheckedChanged(object sender, EventArgs e)
        {
            GetSelectedPinCommandValue(sender);
        }

        private void buttonPerformPinCommand_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (ToolPGPRef.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 入力欄の内容をチェック
            if (CheckForPerformPinCommand() == false) {
                return;
            }

            // PIN番号管理コマンドを実行
            EnableButtons(false);
            DoCommandPinManagement();
        }

        private void buttonClose_Click(object sender, EventArgs e)
        {
            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.Cancel);
        }

        private void TerminateWindow(DialogResult dialogResult)
        {
            // 画面項目を初期化し、この画面を閉じる
            InitFieldValue();
            DialogResult = dialogResult;
            Close();
        }

        //
        // PGP鍵管理タブ関連の処理
        //
        private void InitFieldValue()
        {
            // PGP鍵管理タブ内の入力項目を初期化（このタブが選択状態になります）
            tabPreference.SelectedTab = tabPagePGPKeyManagement;
            InitTabPGPKeyManagement();

            // PIN番号管理タブ内の入力項目を初期化
            InitTabPinManagement();
        }

        private void InitTabPGPKeyManagement()
        {
            // テキストボックスの初期化
            InitTabPGPKeyPathFields();
            InitTabPGPKeyEntryFields();
        }

        private void InitTabPGPKeyPathFields()
        {
            // ファイルパスのテキストボックスを初期化
            textPubkeyFolderPath.Text = "";
            textBackupFolderPath.Text = "";
        }

        private void InitTabPGPKeyEntryFields()
        {
            // テキストボックスを初期化
            textRealName.Text = "";
            textMailAddress.Text = "";
            textComment.Text = "";
            textPin.Text = "";
            textPinConfirm.Text = "";

            // テキストボックスのカーソルを先頭の項目に配置
            textRealName.Focus();
        }

        void EnableButtons(bool enabled)
        {
            // ボタンや入力欄の使用可能／不可制御
            buttonClose.Enabled = enabled;
            buttonPGPStatus.Enabled = enabled;
            buttonPGPReset.Enabled = enabled;
            buttonFirmwareReset.Enabled = enabled;

            // 現在選択中のタブ内も同様に制御を行う
            TabPage page = tabPreference.SelectedTab;
            if (page.Equals(tabPagePGPKeyManagement)) {
                EnableButtonsInTabPGPKeyManagement(enabled);
            }
            if (page.Equals(tabPagePinManagement)) {
                EnableButtonsInTabPinManagement(enabled);
            }
        }

        void EnableButtonsInTabPGPKeyManagement(bool enabled)
        {
            // ボタンや入力欄の使用可能／不可制御
            textRealName.Enabled = enabled;
            textMailAddress.Enabled = enabled;
            textComment.Enabled = enabled;
            textPin.Enabled = enabled;
            textPinConfirm.Enabled = enabled;

            buttonPubkeyFolderPath.Enabled = enabled;
            buttonBackupFolderPath.Enabled = enabled;
            buttonInstallPGPKey.Enabled = enabled;
        }

        //
        // PIN番号管理タブ関連の処理
        //
        private AppCommon.RequestType SelectedPinCommand { get; set; }
        private string SelectedPinCommandName { get; set; }

        private void InitTabPinManagement()
        {
            // ラジオボタンの初期化
            InitButtonPinCommandsWithDefault(radioButton1);
        }

        private void InitButtonPinCommandsWithDefault(RadioButton radioButton)
        {
            // 「実行する機能」のラジオボタン「PIN番号を変更」を選択状態にする
            radioButton.Checked = true;
            GetSelectedPinCommandValue(radioButton);
        }

        private void InitTabPinManagementPinFields()
        {
            // PIN番号のテキストボックスを初期化
            textCurPin.Text = "";
            textNewPin.Text = "";
            textNewPinConf.Text = "";

            // テキストボックスのカーソルを先頭の項目に配置
            textCurPin.Focus();
        }

        void EnableButtonsInTabPinManagement(bool enabled)
        {
            // ボタンや入力欄の使用可能／不可制御
            groupBoxPinCommand.Enabled = enabled;
            groupBoxPinText.Enabled = enabled;
            buttonPerformPinCommand.Enabled = enabled;
        }

        private void GetSelectedPinCommandValue(object sender)
        {
            // ラジオボタンの選択状態に応じ、入力欄のキャプションも変更する
            if (sender.Equals(radioButton1)) {
                // PIN番号を変更
                SelectedPinCommand = AppCommon.RequestType.OpenPGPChangePin;
                SelectedPinCommandName = AppCommon.MSG_LABEL_COMMAND_OPENPGP_CHANGE_PIN;
                labelCurPin.Text = AppCommon.MSG_LABEL_ITEM_CUR_PIN;
                labelNewPin.Text = AppCommon.MSG_LABEL_ITEM_NEW_PIN;
            }
            if (sender.Equals(radioButton2)) {
                // 管理用PIN番号を変更
                SelectedPinCommand = AppCommon.RequestType.OpenPGPChangeAdminPin;
                SelectedPinCommandName = AppCommon.MSG_LABEL_COMMAND_OPENPGP_CHANGE_ADMIN_PIN;
                labelCurPin.Text = AppCommon.MSG_LABEL_ITEM_CUR_ADMPIN;
                labelNewPin.Text = AppCommon.MSG_LABEL_ITEM_NEW_ADMPIN;
            }
            if (sender.Equals(radioButton3)) {
                // PIN番号をリセット
                SelectedPinCommand = AppCommon.RequestType.OpenPGPUnblockPin;
                SelectedPinCommandName = AppCommon.MSG_LABEL_COMMAND_OPENPGP_UNBLOCK_PIN;
                labelCurPin.Text = AppCommon.MSG_LABEL_ITEM_CUR_ADMPIN;
                labelNewPin.Text = AppCommon.MSG_LABEL_ITEM_NEW_PIN;
            }
            if (sender.Equals(radioButton4)) {
                // リセットコードを変更
                SelectedPinCommand = AppCommon.RequestType.OpenPGPSetResetCode;
                SelectedPinCommandName = AppCommon.MSG_LABEL_COMMAND_OPENPGP_SET_RESET_CODE;
                labelCurPin.Text = AppCommon.MSG_LABEL_ITEM_CUR_ADMPIN;
                labelNewPin.Text = AppCommon.MSG_LABEL_ITEM_NEW_RESET_CODE;
            }
            if (sender.Equals(radioButton5)) {
                // リセットコードでPIN番号をリセット
                SelectedPinCommand = AppCommon.RequestType.OpenPGPUnblock;
                SelectedPinCommandName = AppCommon.MSG_LABEL_COMMAND_OPENPGP_UNBLOCK;
                labelCurPin.Text = AppCommon.MSG_LABEL_ITEM_CUR_RESET_CODE;
                labelNewPin.Text = AppCommon.MSG_LABEL_ITEM_NEW_PIN;
            }

            // 確認欄のキャプションを設定
            labelNewPinConf.Text = string.Format(AppCommon.MSG_FORMAT_OPENPGP_ITEM_FOR_CONF, labelNewPin.Text);

            // PIN入力欄をクリアし、新しいPIN欄にフォーカスを移す
            InitTabPinManagementPinFields();
        }

        //
        // 入力チェック関連
        //
        private bool CheckForInstallPGPKey()
        {
            // 入力欄のチェック
            if (CheckMustEntry(textRealName, AppCommon.MSG_LABEL_PGP_REAL_NAME, OPENPGP_NAME_SIZE_MIN, OPENPGP_ENTRY_SIZE_MAX) == false) {
                return false;
            }
            if (CheckAsciiEntry(textRealName, AppCommon.MSG_LABEL_PGP_REAL_NAME) == false) {
                return false;
            }
            if (CheckEntryNoSpaceExistOnBothEnds(textRealName, AppCommon.MSG_LABEL_PGP_REAL_NAME) == false) {
                return false;
            }
            if (CheckMustEntry(textMailAddress, AppCommon.MSG_LABEL_PGP_MAIL_ADDRESS, 1, OPENPGP_ENTRY_SIZE_MAX) == false) {
                return false;
            }
            if (CheckAddressEntry(textMailAddress, AppCommon.MSG_LABEL_PGP_MAIL_ADDRESS) == false) {
                return false;
            }
            if (CheckMustEntry(textComment, AppCommon.MSG_LABEL_PGP_COMMENT, 1, OPENPGP_ENTRY_SIZE_MAX) == false) {
                return false;
            }
            if (CheckAsciiEntry(textComment, AppCommon.MSG_LABEL_PGP_COMMENT) == false) {
                return false;
            }
            if (CheckEntryNoSpaceExistOnBothEnds(textComment, AppCommon.MSG_LABEL_PGP_COMMENT) == false) {
                return false;
            }
            if (CheckPathEntry(textPubkeyFolderPath, AppCommon.MSG_PROMPT_SELECT_PGP_PUBKEY_FOLDER) == false) {
                return false;
            }
            if (CheckPathEntry(textBackupFolderPath, AppCommon.MSG_PROMPT_SELECT_PGP_BACKUP_FOLDER) == false) {
                return false;
            }
            if (CheckPinNumber(textPin, AppCommon.MSG_LABEL_PGP_ADMIN_PIN) == false) {
                return false;
            }
            if (CheckPinNumber(textPinConfirm, AppCommon.MSG_LABEL_PGP_ADMIN_PIN_CONFIRM) == false) {
                return false;
            }

            // 確認用PINコードのチェック
            if (CheckPinConfirm(textPinConfirm, textPin, AppCommon.MSG_LABEL_PGP_ADMIN_PIN_CONFIRM) == false) {
                return false;
            }

            // プロンプトを表示し、Yesの場合だけ処理を行う
            return FormUtil.DisplayPromptPopup(this, AppCommon.MSG_OPENPGP_INSTALL_PGP_KEY, AppCommon.MSG_PROMPT_INSTALL_PGP_KEY);
        }

        private bool CheckMustEntry(TextBox text, string fieldName, int sizeMin, int sizeMax)
        {
            // 必須チェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_MUST_ENTRY, fieldName);
            if (text.Text.Length == 0) {
                FormUtil.ShowWarningMessage(this, MainForm.MaintenanceToolTitle, informativeText);
                text.Focus();
                return false;
            }

            // 長さチェック
            informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ENTRY_DIGIT, fieldName, sizeMin, sizeMax);
            if (FormUtil.CheckEntrySize(text, sizeMin, sizeMax, MainForm.MaintenanceToolTitle, informativeText) == false) {
                return false;
            }

            return true;
        }

        private bool CheckAsciiEntry(TextBox text, string fieldName)
        {
            // 入力パターンチェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ASCII_ENTRY, fieldName);
            if (FormUtil.CheckValueWithPattern(text, OPENPGP_ENTRY_PATTERN_ASCII, MainForm.MaintenanceToolTitle, informativeText) == false) {
                return false;
            }
            return true;
        }

        private bool CheckAddressEntry(TextBox text, string fieldName)
        {
            // 入力パターンチェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ADDRESS_ENTRY, fieldName);
            if (FormUtil.CheckValueWithPattern(text, OPENPGP_ENTRY_PATTERN_MAIL_ADDRESS, MainForm.MaintenanceToolTitle, informativeText) == false) {
                return false;
            }
            return true;
        }

        private bool CheckEntryNoSpaceExistOnBothEnds(TextBox text, string fieldName)
        {
            // 先頭または末尾に半角スペース文字が入っている場合はエラー
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ENTRY_NOSP_BOTH_ENDS, fieldName);
            if (FormUtil.CheckValueWithPattern(text, OPENPGP_ENTRY_PATTERN_NOSP_BOTH_ENDS, MainForm.MaintenanceToolTitle, informativeText) == false) {
                return false;
            }
            return true;
        }

        private bool CheckPathEntry(TextBox text, string messageIfError)
        {
            // 必須チェック（ただし、入力できないのでフォーカスは移動しない）
            if (text.Text.Length == 0) {
                FormUtil.ShowWarningMessage(this, MainForm.MaintenanceToolTitle, messageIfError);
                return false;
            }

            // 入力されたファイルパスが存在しない場合は終了
            string path = text.Text;
            if (Directory.Exists(path) == false) {
                FormUtil.ShowWarningMessage(this, MainForm.MaintenanceToolTitle, messageIfError);
                return false;
            }

            return true;
        }

        private bool CheckPinNumber(TextBox text, string fieldName)
        {
            // 長さチェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ADMIN_PIN_DIGIT, fieldName);
            if (FormUtil.CheckEntrySize(text, OPENPGP_ADMIN_PIN_CODE_SIZE_MIN, OPENPGP_ADMIN_PIN_CODE_SIZE_MAX, MainForm.MaintenanceToolTitle, informativeText) == false) {
                return false;
            }

            // 数字チェック
            informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ADMIN_PIN_NUM, fieldName);
            if (FormUtil.CheckIsNumeric(text, MainForm.MaintenanceToolTitle, informativeText) == false) {
                return false;
            }

            return true;
        }

        private bool CheckPinConfirm(TextBox textPinConfirm, TextBox textPin, string fieldName)
        {
            // PIN番号の確認入力内容をチェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ADMIN_PIN_CONFIRM, fieldName);
            return FormUtil.CompareEntry(textPinConfirm, textPin, MainForm.MaintenanceToolTitle, informativeText);
        }

        private bool CheckForPerformPinCommand()
        {
            // チェック用パラメーターの設定
            string msgCurPin = "";
            string msgNewPin = "";
            int minSizeCurPin = 0;
            int minSizeNewPin = 0;
            switch (SelectedPinCommand) {
            case AppCommon.RequestType.OpenPGPChangePin:
                msgCurPin = AppCommon.MSG_LABEL_ITEM_CUR_PIN;
                minSizeCurPin = 6;
                msgNewPin = AppCommon.MSG_LABEL_ITEM_NEW_PIN;
                minSizeNewPin = 6;
                break;
            case AppCommon.RequestType.OpenPGPChangeAdminPin:
                msgCurPin = AppCommon.MSG_LABEL_ITEM_CUR_ADMPIN;
                minSizeCurPin = 8;
                msgNewPin = AppCommon.MSG_LABEL_ITEM_NEW_ADMPIN;
                minSizeNewPin = 8;
                break;
            case AppCommon.RequestType.OpenPGPUnblockPin:
                msgCurPin = AppCommon.MSG_LABEL_ITEM_CUR_ADMPIN;
                minSizeCurPin = 8;
                msgNewPin = AppCommon.MSG_LABEL_ITEM_NEW_PIN;
                minSizeNewPin = 6;
                break;
            case AppCommon.RequestType.OpenPGPSetResetCode:
                msgCurPin = AppCommon.MSG_LABEL_ITEM_CUR_ADMPIN;
                minSizeCurPin = 8;
                msgNewPin = AppCommon.MSG_LABEL_ITEM_NEW_RESET_CODE;
                minSizeNewPin = 8;
                break;
            case AppCommon.RequestType.OpenPGPUnblock:
                msgCurPin = AppCommon.MSG_LABEL_ITEM_CUR_RESET_CODE;
                minSizeCurPin = 8;
                msgNewPin = AppCommon.MSG_LABEL_ITEM_NEW_PIN;
                minSizeNewPin = 6;
                break;
            default:
                break;
            }

            // 現在のPINをチェック
            if (CheckPinNumberForPinCommand(textCurPin, msgCurPin, minSizeCurPin) == false) {
                return false;
            }

            // 新しいPINをチェック
            if (CheckPinNumberForPinCommand(textNewPin, msgNewPin, minSizeNewPin) == false) {
                return false;
            }

            // 確認用PINのラベル
            string msgNewPinConf = string.Format(AppCommon.MSG_FORMAT_OPENPGP_ITEM_FOR_CONFIRM, msgNewPin);

            // 確認用PINをチェック
            if (CheckPinConfirm(textNewPinConf, textNewPin, msgNewPinConf) == false) {
                return false;
            }

            // プロンプトを表示し、Yesの場合だけ処理を行う
            string caption = string.Format(AppCommon.MSG_FORMAT_OPENPGP_WILL_PROCESS, SelectedPinCommandName);
            return FormUtil.DisplayPromptPopup(this, caption, AppCommon.MSG_PROMPT_OPENPGP_PIN_COMMAND);
        }

        private bool CheckPinNumberForPinCommand(TextBox text, string fieldName, int size_min)
        {
            // 長さチェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_PIN_DIGIT, fieldName, size_min);
            if (FormUtil.CheckEntrySize(text, size_min, size_min, MainForm.MaintenanceToolTitle, informativeText) == false) {
                return false;
            }

            // 数字チェック
            informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ADMIN_PIN_NUM, fieldName);
            if (FormUtil.CheckIsNumeric(text, MainForm.MaintenanceToolTitle, informativeText) == false) {
                return false;
            }

            return true;
        }

        //
        // OpenPGP設定機能の各処理
        //
        void DoCommandInstallPGPKey()
        {
            parameter.RealName = textRealName.Text;
            parameter.MailAddress = textMailAddress.Text;
            parameter.Comment = textComment.Text;
            parameter.Passphrase = textPinConfirm.Text;
            parameter.PubkeyFolderPath = textPubkeyFolderPath.Text;
            parameter.BackupFolderPath = textBackupFolderPath.Text;
            ToolPGPRef.DoOpenPGPCommand(AppCommon.RequestType.OpenPGPInstallKeys, parameter);
        }

        void DoCommandPGPStatus()
        {
            ToolPGPRef.DoOpenPGPCommand(AppCommon.RequestType.OpenPGPStatus, null);
        }

        void DoCommandPGPReset()
        {
            ToolPGPRef.DoOpenPGPCommand(AppCommon.RequestType.OpenPGPReset, null);
        }

        void DoCommandResetFirmware()
        {
            ToolPGPRef.DoCommandResetFirmware();
        }

        private void DoCommandPinManagement()
        {
            // 入力PINをパラメーターとして、コマンドを実行
            ToolPGPParameter parameter = new ToolPGPParameter();
            parameter.CurrentPin = textCurPin.Text;
            parameter.NewPin = textNewPin.Text;
            parameter.NewPinForConfirm = textNewPinConf.Text;
            parameter.SelectedPinCommand = SelectedPinCommand;
            parameter.SelectedPinCommandName = SelectedPinCommandName;
            ToolPGPRef.DoOpenPGPCommand(SelectedPinCommand, parameter);
        }

        public void OnCommandProcessTerminated(AppCommon.RequestType requestType, bool success, string errMessage)
        {
            // 処理終了メッセージをポップアップ表示後、画面項目を使用可とする
            DisplayResultMessage(requestType, success, errMessage);
            ClearEntry(requestType, success);
            EnableButtons(true);
        }

        private void DisplayResultMessage(AppCommon.RequestType requestType, bool success, string errMessage)
        {
            // 処理名称を設定
            string name = "";
            switch (requestType) {
                case AppCommon.RequestType.OpenPGPInstallKeys:
                    name = AppCommon.MSG_LABEL_COMMAND_OPENPGP_INSTALL_KEYS;
                    break;
                case AppCommon.RequestType.OpenPGPStatus:
                    if (success) {
                        // メッセージの代わりに、OpenPGP設定情報を、情報表示画面に表示
                        CommonDisplayInfoForm.OpenForm(this, AppCommon.PROCESS_NAME_OPENPGP_STATUS, ToolPGPRef.GetPGPStatusInfoString());
                        return;
                    }
                    name = AppCommon.MSG_LABEL_COMMAND_OPENPGP_STATUS;
                    break;
                case AppCommon.RequestType.OpenPGPReset:
                    name = AppCommon.MSG_LABEL_COMMAND_OPENPGP_RESET;
                    break;
                case AppCommon.RequestType.HidFirmwareReset:
                    name = AppCommon.PROCESS_NAME_FIRMWARE_RESET;
                    break;
                case AppCommon.RequestType.OpenPGPChangePin:
                case AppCommon.RequestType.OpenPGPChangeAdminPin:
                case AppCommon.RequestType.OpenPGPUnblockPin:
                case AppCommon.RequestType.OpenPGPSetResetCode:
                case AppCommon.RequestType.OpenPGPUnblock:
                    name = SelectedPinCommandName;
                    break;
                default:
                    break;
            }

            // メッセージをポップアップ表示
            string formatted = string.Format(AppCommon.MSG_FORMAT_END_MESSAGE,
                name,
                success ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE);
            if (success) {
                FormUtil.ShowInfoMessage(this, MainForm.MaintenanceToolTitle, formatted);
            } else {
                FormUtil.ShowWarningMessage(this, formatted, errMessage);
            }
        }

        private void ClearEntry(AppCommon.RequestType requestType, bool success)
        {
            // 全ての入力欄をクリア
            switch (requestType) {
                case AppCommon.RequestType.OpenPGPInstallKeys:
                    if (success) {
                        InitTabPGPKeyPathFields();
                        InitTabPGPKeyEntryFields();
                    }
                    break;
                case AppCommon.RequestType.OpenPGPChangePin:
                case AppCommon.RequestType.OpenPGPChangeAdminPin:
                case AppCommon.RequestType.OpenPGPUnblockPin:
                case AppCommon.RequestType.OpenPGPSetResetCode:
                case AppCommon.RequestType.OpenPGPUnblock:
                    if (success) {
                        InitTabPinManagementPinFields();
                    }
                    break;
                default:
                    break;
            }
        }
    }
}
