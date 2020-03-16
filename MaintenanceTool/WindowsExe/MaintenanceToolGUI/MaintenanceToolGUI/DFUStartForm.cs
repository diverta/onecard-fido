using System;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class DFUStartForm : Form
    {
        // DFU処理クラスの参照を保持
        private ToolDFU ToolDFURef;

        public DFUStartForm()
        {
            InitializeComponent();
        }

        public bool OpenForm(ToolDFU td)
        {
            // DFU処理クラスを参照し、バージョン文字列をラベルに設定
            ToolDFURef = td;
            LabelUpdateVersion.Text = ToolDFURef.UpdateVersion;
            LabelCurrentVersion.Text = ToolDFURef.CurrentVersion;

            // 処理開始画面を開く
            DialogResult = DialogResult.Cancel;
            return (ShowDialog() == DialogResult.OK);
        }

        private void TerminateWindow()
        {
            // 処理開始画面を閉じる
            Close();
        }

        private void ButtonOK_Click(object sender, EventArgs e)
        {
            DialogResult = DialogResult.OK;
            TerminateWindow();
        }

        private void ButtonCancel_Click(object sender, EventArgs e)
        {
            TerminateWindow();
        }
    }
}
