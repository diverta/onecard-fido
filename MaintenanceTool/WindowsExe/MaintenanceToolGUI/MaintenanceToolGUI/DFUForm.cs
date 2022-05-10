using System;
using System.Windows.Forms;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    public partial class DFUForm : Form
    {
        // 親画面の参照を保持
        private MainForm mainForm;

        // 入力されたパラメーターを保持
        public string CommandTitle = "";

        public DFUForm(MainForm mf)
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

        private void buttonUSBDFU_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 画面入力値をパラメーターに保持
            CommandTitle = ToolGUICommon.PROCESS_NAME_USB_DFU;

            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.OK);
        }

        private void buttonBLEDFU_Click(object sender, EventArgs e)
        {
            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}",
                ToolGUICommon.MSG_PROMPT_START_BLE_DFU_PROCESS,
                ToolGUICommon.MSG_COMMENT_START_BLE_DFU_PROCESS);

            // プロンプトを表示し、Yesの場合だけ処理を続行する
            if (FormUtil.DisplayPromptPopup(this, MainForm.MaintenanceToolTitle, message) == false) {
                return;
            }

            // 画面入力値をパラメーターに保持
            CommandTitle = ToolGUICommon.PROCESS_NAME_BLE_DFU;

            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.OK);
        }
    }
}
