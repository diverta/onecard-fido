using System;
using System.Windows.Forms;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    public partial class DFUNewStartForm : Form
    {
        // DFU処理クラスの参照を保持
        private ToolDFU ToolDFURef;

        public DFUNewStartForm(ToolDFU td)
        {
            InitializeComponent();
            ToolDFURef = td;

            // メッセージ文言を設定
            LabelComment.Text = string.Format("{0}\n\n{1}",
                ToolGUICommon.MSG_COMMENT_START_DFU_PROCESS,
                ToolGUICommon.MSG_PROMPT_START_DFU_PROCESS);
        }

        public bool OpenForm(IWin32Window owner)
        {
            // 処理開始画面を開く
            DialogResult = DialogResult.Cancel;
            return (ShowDialog(owner) == DialogResult.OK);
        }

        private void ButtonOK_Click(object sender, EventArgs e)
        {
            // ボタンを押下不可にし、DFU対象デバイスに接続
            EnableButtons(false);
            ToolDFURef.EstablishDFUConnection();
        }

        private void ButtonCancel_Click(object sender, EventArgs e)
        {
            TerminateWindow();
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

        //
        // DFU処理用のインターフェース
        //
        public void OnDFUConnectionEstablished(bool success, string captionMessage, string errorMessage)
        {
            if (success) {
                // 接続処理がOKの場合
                DialogResult = DialogResult.OK;
            } else {
                // 接続処理がNGの場合、エラーメッセージをポップアップ表示
                FormUtil.ShowWarningMessage(this, captionMessage, errorMessage);
            }
            // このウィンドウを終了
            TerminateWindow();
        }
    }
}
