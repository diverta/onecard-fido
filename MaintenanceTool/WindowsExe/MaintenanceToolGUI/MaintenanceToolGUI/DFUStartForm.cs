using System;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class DFUStartForm : Form
    {
        // DFU処理クラスの参照を保持
        private ToolDFU ToolDFURef;

        public DFUStartForm(ToolDFU td)
        {
            InitializeComponent();
            ToolDFURef = td;
        }

        public bool OpenForm()
        {
            // DFU処理クラスを参照し、バージョン文字列をラベルに設定
            LabelUpdateVersion.Text = ToolDFURef.UpdateVersion;
            LabelCurrentVersion.Text = ToolDFURef.CurrentVersion;

            // 処理開始画面を開く
            DialogResult = DialogResult.Cancel;
            return (ShowDialog() == DialogResult.OK);
        }

        private void TerminateWindow()
        {
            // 処理開始画面を閉じる
            EnableButtons(true);
            Close();
        }

        private void EnableButtons(bool b)
        {
            ButtonOK.Enabled = b;
            ButtonCancel.Enabled = b;
        }

        private void ButtonOK_Click(object sender, EventArgs e)
        {
            if (ToolDFURef.ChangeToBootloaderMode()) {
                // HID接続がある場合は、DFU対象デバイスをブートローダーモードに遷移させる
                EnableButtons(false);
            }
        }

        private void ButtonCancel_Click(object sender, EventArgs e)
        {
            TerminateWindow();
        }

        //
        // DFU処理用のインターフェース
        //
        public void OnChangeToBootloaderMode(bool success, string captionMessage, string errorMessage)
        {
            if (success) {
                // ブートローダーモード遷移処理がOKの場合
                DialogResult = DialogResult.OK;
            } else {
                // ブートローダーモード遷移処理がNGの場合、エラーメッセージをポップアップ表示
                MessageBox.Show(this, errorMessage, captionMessage,
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            // このウィンドウを終了
            TerminateWindow();
        }
    }
}
