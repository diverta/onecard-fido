using System;
using System.Windows.Forms;

namespace DevelopmentToolGUI
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
            mainForm = mf;
        }

        protected override void OnShown(EventArgs e)
        {
            // Cancelボタンにフォーカス
            buttonCancel.Focus();
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            // この画面を閉じる
            TerminateWindow(DialogResult.Cancel);
        }

        private void TerminateWindow(DialogResult dialogResult)
        {
            // この画面を閉じる
            DialogResult = dialogResult;
            Close();
        }

        private void buttonToolVersionInfo_Click(object sender, EventArgs e)
        {
            // 機能名を設定し、画面を閉じる
            CommandTitle = AppCommon.PROCESS_NAME_TOOL_VERSION_INFO;
            TerminateWindow(DialogResult.OK);
        }

        private void buttonViewLogFile_Click(object sender, EventArgs e)
        {
            // 機能名を設定し、画面を閉じる
            CommandTitle = AppCommon.PROCESS_NAME_VIEW_LOG_FILE;
            TerminateWindow(DialogResult.OK);
        }
    }
}
