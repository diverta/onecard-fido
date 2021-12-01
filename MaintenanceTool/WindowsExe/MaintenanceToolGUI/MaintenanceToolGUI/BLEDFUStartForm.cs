using System;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class BLEDFUStartForm : Form
    {
        public BLEDFUStartForm()
        {
            InitializeComponent();
        }

        public bool OpenForm(IWin32Window owner, ToolBLEDFU toolDFURef)
        {
            // DFU処理クラスを参照し、バージョン文字列をラベルに設定
            LabelUpdateVersion.Text = toolDFURef.UpdateVersion;
            LabelCurrentVersion.Text = toolDFURef.CurrentVersion;

            // 処理開始画面を開く
            return (ShowDialog(owner) == DialogResult.OK);
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
            DialogResult = DialogResult.Cancel;
            TerminateWindow();
        }
    }
}
