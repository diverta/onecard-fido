using System;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class UtilityForm : Form
    {
        // 親画面の参照を保持
        private MainForm mainForm;

        // 入力されたパラメーターを保持
        public string CommandTitle = "";

        public UtilityForm(MainForm mf)
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

        private void buttonFlashROMInfo_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 機能名を設定し、画面を閉じる
            CommandTitle = ToolGUICommon.PROCESS_NAME_GET_FLASH_STAT;
            TerminateWindow(DialogResult.OK);
        }

        private void buttonFWVersionInfo_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 機能名を設定し、画面を閉じる
            CommandTitle = ToolGUICommon.PROCESS_NAME_GET_VERSION_INFO;
            TerminateWindow(DialogResult.OK);
        }

        private void buttonToolVersionInfo_Click(object sender, EventArgs e)
        {
            // 機能名を設定し、画面を閉じる
            CommandTitle = ToolGUICommon.PROCESS_NAME_TOOL_VERSION_INFO;
            TerminateWindow(DialogResult.OK);
        }

        private void buttonViewLogFile_Click(object sender, EventArgs e)
        {
            // 機能名を設定し、画面を閉じる
            CommandTitle = ToolGUICommon.PROCESS_NAME_VIEW_LOG_FILE;
            TerminateWindow(DialogResult.OK);
        }
    }
}
