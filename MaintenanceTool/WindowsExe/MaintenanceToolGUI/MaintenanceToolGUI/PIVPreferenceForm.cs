using System;
using System.IO;
using System.Windows.Forms;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    public partial class PIVPreferenceForm : Form
    {
        // 入力可能文字数
        private const int PIV_PIN_CODE_SIZE_MIN = 6;
        private const int PIV_PIN_CODE_SIZE_MAX = 6;

        // 処理クラスの参照を保持
        private ToolPIV ToolPIVRef;

        public PIVPreferenceForm(ToolPIV toolPIV)
        {
            // 画面項目の初期化
            InitializeComponent();
            InitFieldValue();

            // 処理クラスの参照を保持
            ToolPIVRef = toolPIV;
        }

        private void buttonPkeyFolderPath_Click(object sender, EventArgs e)
        {
            // ファイルを選択
            FormUtil.SelectFilePath(openFileDialog1, AppCommon.MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH, AppCommon.MSG_FILTER_SELECT_PIV_PKEY_PEM_PATH, textPkeyFolderPath);
        }

        private void buttonCertFolderPath_Click(object sender, EventArgs e)
        {
            // ファイルを選択
            FormUtil.SelectFilePath(openFileDialog1, AppCommon.MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH, AppCommon.MSG_FILTER_SELECT_PIV_CERT_PEM_PATH, textCertFolderPath);
        }

        private void buttonInstallPkeyCert_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (ToolPIVRef.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 入力欄の内容をチェック
            if (CheckForInstallPkeyCert()) {
                // 画面入力内容を引数とし、PGP秘密鍵インストール処理を実行
                EnableButtons(false);
                DoCommandInstallPGPKey();
            }
        }

        private void buttonFirmwareReset_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (ToolPIVRef.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 認証器のファームウェアを再起動
            EnableButtons(false);
            DoCommandResetFirmware();
        }

        private void radioPinCommand1_CheckedChanged_1(object sender, EventArgs e)
        {
            GetSelectedPinCommandValue(sender);
        }

        private void radioPinCommand2_CheckedChanged_1(object sender, EventArgs e)
        {
            GetSelectedPinCommandValue(sender);
        }

        private void radioPinCommand3_CheckedChanged_1(object sender, EventArgs e)
        {
            GetSelectedPinCommandValue(sender);
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

        private void InitFieldValue()
        {
            // PGP鍵管理タブ内の入力項目を初期化（このタブが選択状態になります）
            tabPreference.SelectedTab = tabPagePkeyCertManagement;
            InitTabPagePkeyCertManagement();

            // PIN番号管理タブ内の入力項目を初期化
            InitTabPinManagement();
        }

        private void EnableButtons(bool enabled)
        {
            // ボタンや入力欄の使用可能／不可制御
            buttonClose.Enabled = enabled;
            buttonPivStatus.Enabled = enabled;
            buttonInitialSetting.Enabled = enabled;
            buttonClearSetting.Enabled = enabled;
            buttonFirmwareReset.Enabled = enabled;

            // 現在選択中のタブ内も同様に制御を行う
            TabPage page = tabPreference.SelectedTab;
            if (page.Equals(tabPagePkeyCertManagement)) {
                EnableButtonsInTabPagePkeyCertManagement(enabled);
            }
            if (page.Equals(tabPagePinManagement)) {
                EnableButtonsInTabPinManagement(enabled);
            }
        }

        //
        // 鍵・証明書管理タブ関連の処理
        //
        private void InitTabPagePkeyCertManagement()
        {
            // テキストボックスの初期化
            InitTabPkeyCertPathFields();
            InitTabPGPKeyEntryFields();
        }

        private void InitTabPkeyCertPathFields()
        {
            // ファイルパスのテキストボックスを初期化
            textPkeyFolderPath.Text = "";
            textCertFolderPath.Text = "";
        }

        private void InitTabPGPKeyEntryFields()
        {
            // テキストボックスを初期化
            textPin.Text = "";
            textPinConfirm.Text = "";

            // テキストボックスのカーソルを先頭の項目に配置
            textPin.Focus();
        }

        void EnableButtonsInTabPagePkeyCertManagement(bool enabled)
        {
            // ボタンや入力欄の使用可能／不可制御
            textPin.Enabled = enabled;
            textPinConfirm.Enabled = enabled;

            buttonPkeyFolderPath.Enabled = enabled;
            buttonCertFolderPath.Enabled = enabled;
            buttonInstallPkeyCert.Enabled = enabled;
        }

        //
        // PIN番号管理タブ関連の処理
        //
        private AppCommon.RequestType SelectedPinCommand { get; set; }
        private string SelectedPinCommandName { get; set; }

        private void InitTabPinManagement()
        {
            // ラジオボタンの初期化
            InitButtonPinCommandsWithDefault(radioPinCommand1);
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
            if (sender.Equals(radioPinCommand1)) {
                // PIN番号を変更
                SelectedPinCommand = AppCommon.RequestType.PIVChangePin;
                SelectedPinCommandName = AppCommon.MSG_PIV_CHANGE_PIN_NUMBER;
                labelCurPin.Text = AppCommon.MSG_LABEL_CURRENT_PIN;
                labelNewPin.Text = AppCommon.MSG_LABEL_NEW_PIN;
            }
            if (sender.Equals(radioPinCommand2)) {
                // PUK番号を変更
                SelectedPinCommand = AppCommon.RequestType.PIVChangePuk;
                SelectedPinCommandName = AppCommon.MSG_PIV_CHANGE_PUK_NUMBER;
                labelCurPin.Text = AppCommon.MSG_LABEL_CURRENT_PUK;
                labelNewPin.Text = AppCommon.MSG_LABEL_NEW_PUK;
            }
            if (sender.Equals(radioPinCommand3)) {
                // PIN番号をリセット
                SelectedPinCommand = AppCommon.RequestType.PivUnblockPin;
                SelectedPinCommandName = AppCommon.MSG_PIV_RESET_PIN_NUMBER;
                labelCurPin.Text = AppCommon.MSG_LABEL_CURRENT_PUK;
                labelNewPin.Text = AppCommon.MSG_LABEL_NEW_PIN;
            }

            // 確認欄のキャプションを設定
            labelNewPinConf.Text = string.Format(AppCommon.MSG_FORMAT_OPENPGP_ITEM_FOR_CONF, labelNewPin.Text);

            // PIN入力欄をクリアし、新しいPIN欄にフォーカスを移す
            InitTabPinManagementPinFields();
        }

        //
        // 入力チェック関連
        //
        private bool CheckForInstallPkeyCert()
        {
            // 入力欄のチェック
            if (CheckPathEntry(textPkeyFolderPath, AppCommon.MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH) == false) {
                return false;
            }
            if (CheckPathEntry(textCertFolderPath, AppCommon.MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH) == false) {
                return false;
            }
            if (CheckPinNumber(textPin, AppCommon.MSG_LABEL_CURRENT_PIN) == false) {
                return false;
            }
            if (CheckPinNumber(textPinConfirm, AppCommon.MSG_LABEL_CURRENT_PIN_FOR_CONFIRM) == false) {
                return false;
            }

            // 確認用PINコードのチェック
            if (CheckPinConfirm(textPinConfirm, textPin, AppCommon.MSG_LABEL_CURRENT_PIN_FOR_CONFIRM) == false) {
                return false;
            }

            // プロンプトを表示し、Yesの場合だけ処理を行う
            return FormUtil.DisplayPromptPopup(this, AppCommon.MSG_INSTALL_PIV_PKEY_CERT, AppCommon.MSG_PROMPT_INSTALL_PGP_KEY);
        }

        private bool CheckPathEntry(TextBox text, string messageIfError)
        {
            // 必須チェック（ただし、入力できないのでフォーカスは移動しない）
            if (text.Text.Length == 0) {
                FormUtil.ShowWarningMessage(this, MainForm.MaintenanceToolTitle, messageIfError);
                return false;
            }

            // 入力されたファイルが存在しない場合は終了
            string path = text.Text;
            if (File.Exists(path) == false) {
                FormUtil.ShowWarningMessage(this, MainForm.MaintenanceToolTitle, messageIfError);
                return false;
            }

            return true;
        }

        private bool CheckPinNumber(TextBox text, string fieldName)
        {
            // 長さチェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PIV_PIN_DIGIT, fieldName);
            if (FormUtil.CheckEntrySize(text, PIV_PIN_CODE_SIZE_MIN, PIV_PIN_CODE_SIZE_MAX, MainForm.MaintenanceToolTitle, informativeText) == false) {
                return false;
            }

            // 数字チェック
            informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PIV_PIN_NUM, fieldName);
            if (FormUtil.CheckIsNumeric(text, MainForm.MaintenanceToolTitle, informativeText) == false) {
                return false;
            }

            return true;
        }

        private bool CheckPinConfirm(TextBox textPinConfirm, TextBox textPin, string fieldName)
        {
            // PIN番号の確認入力内容をチェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PIV_PIN_CONFIRM, fieldName);
            return FormUtil.CompareEntry(textPinConfirm, textPin, MainForm.MaintenanceToolTitle, informativeText);
        }

        //
        // PIV設定機能の各処理
        //
        void DoCommandInstallPGPKey()
        {
            // TODO:
            // 画面入力内容をパラメーターとして、鍵／証明書インストール処理を実行
        }

        void DoCommandResetFirmware()
        {
            ToolPIVRef.DoCommandResetFirmware();
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
            case AppCommon.RequestType.HidFirmwareReset:
                name = AppCommon.PROCESS_NAME_FIRMWARE_RESET;
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
            default:
                break;
            }
        }
    }
}
