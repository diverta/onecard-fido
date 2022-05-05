using MaintenanceToolCommon;
using System;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class BLEForm : Form
    {
        // 親画面の参照を保持
        private MainForm mainForm;

        // ペアリング開始画面
        private PairingStartForm PairingStartFormRef;

        // 入力されたパラメーターを保持
        public string CommandTitle = "";
        private string Passkey;

        public BLEForm(MainForm mf)
        {
            InitializeComponent();
            InitFieldValue();
            mainForm = mf;

            // ペアリング開始画面を生成
            PairingStartFormRef = new PairingStartForm();
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

        private void buttonPairing_Click(object sender, EventArgs e)
        {
            // ペアリング開始画面を表示
            if (PairingStartFormRef.OpenForm(this) == false) {
                // Cancelクリックの場合は処理中止
                return;
            }

            // ペアリング開始画面で入力したパスキーを保持
            Passkey = PairingStartFormRef.GetPasskey();

            // 画面入力値をパラメーターに保持
            CommandTitle = ToolGUICommon.PROCESS_NAME_PAIRING;

            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.OK);
        }

        private void buttonUnpairing_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 確認メッセージを表示し、Yesの場合だけ処理を続行する
            string message = string.Format("{0}\n\n{1}",
                AppCommon.MSG_ERASE_BONDS,
                AppCommon.MSG_PROMPT_ERASE_BONDS);
            if (FormUtil.DisplayPromptPopup(this, message) == false) {
                return;
            }

            // 画面入力値をパラメーターに保持
            CommandTitle = ToolGUICommon.PROCESS_NAME_ERASE_BONDS;

            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.OK);
        }

        public string GetPasskey()
        {
            return Passkey;
        }
    }
}
