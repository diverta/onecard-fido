using System;
using System.Windows.Forms;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    public partial class FIDOAttestationForm : Form
    {
        // 親画面の参照を保持
        private MainForm mainForm;

        // 入力されたパラメーターを保持
        public string CommandTitle = "";
        public string KeyPath = "";
        public string CertPath = "";

        public FIDOAttestationForm(MainForm mf)
        {
            InitializeComponent();
            InitFieldValue();
            mainForm = mf;
        }

        protected override void OnShown(EventArgs e)
        {
            // Cancelボタンにフォーカス
            buttonCancel.Focus();
        }

        private void buttonCancel_Click(object sender, EventArgs e)
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
            // 画面項目を初期値に設定
            textKeyPath.Text = "";
            textCertPath.Text = "";
        }

        private bool checkPathEntry(TextBox textBox, string errorMessage)
        {
            // 入力済みの場合はチェックOK
            if (textBox.Text.Length > 0) {
                return true;
            }

            // 未入力の場合はポップアップメッセージを表示して
            // テキストボックスにフォーカスを移す
            FormUtil.ShowWarningMessage(this, MainForm.GetMaintenanceToolTitle(), errorMessage);
            textBox.Focus();

            return false;
        }

        private void buttonSelectKeyPath_Click(object sender, EventArgs e)
        {
            FormUtil.SelectFilePath(openFileDialog1,
                ToolGUICommon.MSG_PROMPT_SELECT_PKEY_PATH,
                ToolGUICommon.FILTER_SELECT_PEM_PATH,
                textKeyPath);
        }

        private void buttonSelectCertPath_Click(object sender, EventArgs e)
        {
            FormUtil.SelectFilePath(openFileDialog1,
                ToolGUICommon.MSG_PROMPT_SELECT_CRT_PATH,
                ToolGUICommon.FILTER_SELECT_CRT_PATH,
                textCertPath);
        }

        private void buttonInstall_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }
            // ファイルパス入力チェック
            if (checkPathEntry(textKeyPath, ToolGUICommon.MSG_PROMPT_SELECT_PKEY_PATH) == false) {
                return;
            }
            if (checkPathEntry(textCertPath, ToolGUICommon.MSG_PROMPT_SELECT_CRT_PATH) == false) {
                return;
            }
            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}",
                ToolGUICommon.MSG_INSTALL_SKEY_CERT,
                ToolGUICommon.MSG_PROMPT_INSTL_SKEY_CERT);

            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (FormUtil.DisplayPromptPopup(this, MainForm.MaintenanceToolTitle, message) == false) {
                return;
            }

            // 画面入力値をパラメーターに保持
            KeyPath = textKeyPath.Text;
            CertPath = textCertPath.Text;
            CommandTitle = ToolGUICommon.PROCESS_NAME_INSTALL_SKEY_CERT;

            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.OK);
        }

        private void buttonDelete_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }
            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}",
                ToolGUICommon.MSG_ERASE_SKEY_CERT,
                ToolGUICommon.MSG_PROMPT_ERASE_SKEY_CERT);

            // 鍵・証明書削除
            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (FormUtil.DisplayPromptPopup(this, MainForm.MaintenanceToolTitle, message) == false) {
                return;
            }

            // 画面入力値をパラメーターに保持
            KeyPath = "";
            CertPath = "";
            CommandTitle = ToolGUICommon.PROCESS_NAME_ERASE_SKEY_CERT;

            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.OK);
        }
    }
}
