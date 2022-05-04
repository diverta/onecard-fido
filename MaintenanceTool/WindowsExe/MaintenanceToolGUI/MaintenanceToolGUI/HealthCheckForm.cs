using System;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class HealthCheckForm : Form
    {
        // 親画面の参照を保持
        private MainForm mainForm;

        // 入力されたパラメーターを保持
        public string CommandTitle = "";

        public HealthCheckForm(MainForm mf)
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

        private void buttonBLECtap2HealthCheck_Click(object sender, EventArgs e)
        {
            // 機能名を設定し、画面を閉じる
            CommandTitle = ToolGUICommon.PROCESS_NAME_BLE_CTAP2_HEALTHCHECK;
            TerminateWindow(DialogResult.OK);
        }

        private void buttonBLEU2FHealthCheck_Click(object sender, EventArgs e)
        {
            // 機能名を設定し、画面を閉じる
            CommandTitle = ToolGUICommon.PROCESS_NAME_BLE_U2F_HEALTHCHECK;
            TerminateWindow(DialogResult.OK);
        }

        private void buttonBLEPingTest_Click(object sender, EventArgs e)
        {
            // 機能名を設定し、画面を閉じる
            CommandTitle = ToolGUICommon.PROCESS_NAME_TEST_BLE_PING;
            TerminateWindow(DialogResult.OK);
        }

        private void buttonHIDCtap2HealthCheck_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 機能名を設定し、画面を閉じる
            CommandTitle = ToolGUICommon.PROCESS_NAME_HID_CTAP2_HEALTHCHECK;
            TerminateWindow(DialogResult.OK);
        }

        private void buttonHIDU2FHealthCheck_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 機能名を設定し、画面を閉じる
            CommandTitle = ToolGUICommon.PROCESS_NAME_HID_U2F_HEALTHCHECK;
            TerminateWindow(DialogResult.OK);
        }

        private void buttonHIDPingTest_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (mainForm.CheckUSBDeviceDisconnected()) {
                return;
            }

            // 機能名を設定し、画面を閉じる
            CommandTitle = ToolGUICommon.PROCESS_NAME_TEST_CTAPHID_PING;
            TerminateWindow(DialogResult.OK);
        }
    }
}
