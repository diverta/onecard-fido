using System;
using System.Windows.Forms;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    public partial class FIDOForm : Form
    {
        // 親画面の参照を保持
        private MainForm mainForm;

        // 入力されたパラメーターを保持
        public string CommandTitle = "";
        public string PinNew = "";
        public string PinOld = "";
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

        private void buttonSetPinParam_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }

            // パラメーター入力画面を表示
            SetPinParamForm f = new SetPinParamForm();
            if (f.ShowDialog() == DialogResult.Cancel) {
                // パラメーター入力画面でCancelの場合は終了
                return;
            }

            // 画面入力値をパラメーターに保持
            CommandTitle = f.CommandTitle;
            PinNew = f.PinNew;
            PinOld = f.PinOld;

            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.OK);
        }

        private void buttonReset_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }

            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}",
                AppCommon.MSG_CLEAR_PIN_CODE,
                AppCommon.MSG_PROMPT_CLEAR_PIN_CODE);

            // FIDO認証情報の消去（認証器のリセット）
            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (FormUtil.DisplayPromptPopup(this, MainForm.MaintenanceToolTitle, message)) {
                // 画面入力値をパラメーターに保持
                CommandTitle = ToolGUICommon.PROCESS_NAME_AUTH_RESET;

                // 画面項目を初期化し、この画面を閉じる
                TerminateWindow(DialogResult.OK);
            }
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
