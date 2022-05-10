using System;
using System.Windows.Forms;
using ToolGUICommon;

namespace DevelopmentToolGUI
{
    public partial class FIDOForm : Form
    {
        // 親画面の参照を保持
        private MainForm mainForm;

        // 入力されたパラメーターを保持
        public string CommandTitle = "";
        public string KeyPath = "";
        public string CertPath = "";

        public FIDOForm(MainForm mf)
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
        }

        private void buttonReset_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }
            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}",
                AppCommon.MSG_ERASE_SKEY_CERT,
                AppCommon.MSG_PROMPT_ERASE_SKEY_CERT);

            // 鍵・証明書削除
            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (FormUtil.DisplayPromptPopup(this, MainForm.MaintenanceToolTitle, message) == false) {
                return;
            }

            // 画面入力値をパラメーターに保持
            KeyPath = "";
            CertPath = "";
            CommandTitle = AppCommon.PROCESS_NAME_ERASE_SKEY_CERT;

            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.OK);
        }

        private void ButtonFIDOAttestation_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 鍵・証明書設定画面を表示
            FIDOAttestationForm f = new FIDOAttestationForm(mainForm);
            if (f.ShowDialog() == DialogResult.Cancel) {
                // 鍵・証明書設定画面でCancelの場合は終了
                return;
            }

            // 画面入力値をパラメーターに保持
            CommandTitle = f.CommandTitle;
            KeyPath = f.KeyPath;
            CertPath = f.CertPath;

            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.OK);
        }
    }
}
