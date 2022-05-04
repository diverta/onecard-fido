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
        }

        private void buttonBLEU2FHealthCheck_Click(object sender, EventArgs e)
        {
        }

        private void buttonBLEPingTest_Click(object sender, EventArgs e)
        {
        }

        private void buttonHIDCtap2HealthCheck_Click(object sender, EventArgs e)
        {
        }

        private void buttonHIDU2FHealthCheck_Click(object sender, EventArgs e)
        {
        }

        private void buttonHIDPingTest_Click(object sender, EventArgs e)
        {
        }
    }
}
