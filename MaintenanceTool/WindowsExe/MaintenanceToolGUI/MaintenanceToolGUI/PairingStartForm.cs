using System;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class PairingStartForm : Form
    {
        private string Passkey = null;

        public PairingStartForm()
        {
            InitializeComponent();
        }

        public bool OpenForm(IWin32Window owner)
        {
            // 処理開始画面を開く
            DialogResult = DialogResult.Cancel;
            return ShowDialog(owner) == DialogResult.OK;
        }

        private void TerminateWindow()
        {
            // パスキーをクリア
            textPasskey.Text = "";

            // 処理開始画面を閉じる
            Close();
        }

        private void ButtonOK_Click(object sender, EventArgs e)
        {
            // パスキーなしでペアリング実行
            Passkey = null;
            DialogResult = DialogResult.OK;
            TerminateWindow();
        }

        private void ButtonOK_2_Click(object sender, EventArgs e)
        {
            // 入力チェックがNGの場合は中断
            if (CheckEntries() == false) {
                return;
            }

            // パスキーを保持
            Passkey = textPasskey.Text;
            DialogResult = DialogResult.OK;
            TerminateWindow();
        }

        private void ButtonCancel_Click(object sender, EventArgs e)
        {
            // パスキーをクリア
            Passkey = null;
            TerminateWindow();
        }

        private bool CheckEntries()
        {
            // 長さチェックと数字入力チェック
            if (FormUtil.CheckEntrySize(textPasskey, 6, 6, MainForm.MaintenanceToolTitle, ToolGUICommon.MSG_INVALID_FIELD_SIZE) == false) {
                return false;
            }
            if (FormUtil.CheckIsNumeric(textPasskey, MainForm.MaintenanceToolTitle, ToolGUICommon.MSG_INVALID_NUMBER) == false) {
                return false;
            }

            return true;
        }

        public string GetPasskey()
        {
            return Passkey;
        }
    }
}
