using System;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class PGPPreferenceForm : Form
    {
        // 処理クラスの参照を保持
        private ToolPGP ToolPGPRef;

        // 設定パラメーターを保持
        private ToolPGPParameter parameter = new ToolPGPParameter();

        public PGPPreferenceForm(ToolPGP toolPGP)
        {
            // 画面項目の初期化
            InitializeComponent();

            // 処理クラスの参照を保持
            ToolPGPRef = toolPGP;
        }

        private void buttonClose_Click(object sender, EventArgs e)
        {
            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.Cancel);
        }

        private void TerminateWindow(DialogResult dialogResult)
        {
            // 画面項目を初期化し、この画面を閉じる
            DialogResult = dialogResult;
            Close();
        }
    }
}
