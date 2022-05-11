using System;
using System.Windows.Forms;

namespace ToolGUICommon
{
    public partial class ToolVersionForm : Form
    {
        public ToolVersionForm()
        {
            InitializeComponent();
        }

        protected override void OnShown(EventArgs e)
        {
            // 閉じるボタンにフォーカス
            buttonCancel.Focus();
        }

        public void ShowToolVersionDialog(string dialogTitle, string toolName, string toolVersion, string toolCopyright)
        {
            // ダイアログのタイトルを表示
            Text = dialogTitle;

            // ツールタイトル／バージョン／著作権表示
            labelToolName.Text = toolName;
            labelVersion.Text = toolVersion;
            labelCopyright.Text = toolCopyright;

            // この画面を表示
            ShowDialog();
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            // この画面を閉じる
            DialogResult = DialogResult.Cancel;
            Close();
        }
    }
}
