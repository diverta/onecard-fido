using System;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class SetPinParamForm : Form
    {
        // 入力されたパラメーターを保持
        public string PinNew = "";
        public string PinOld = "";
        public string CommandTitle = "";

        public SetPinParamForm()
        {
            InitializeComponent();
            InitFieldValue();
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
            // 画面項目を初期値に設定
            textPin.Text = "";
            textPinConfirm.Text = "";
            textPinOld.Text = "";

            // 最初の項目にフォーカス
            textPin.Focus();
        }

        private void buttonSetPin_Click(object sender, EventArgs e)
        {
            // 入力チェックがNGの場合は中断
            if (CheckEntries() == false) {
                return;
            }

            // 画面入力値をパラメーターに保持
            PinNew = textPin.Text;
            PinOld = "";
            CommandTitle = ToolGUICommon.PROCESS_NAME_CLIENT_PIN_SET;

            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.OK);
        }

        private void buttonChangePin_Click(object sender, EventArgs e)
        {
            // 入力チェックがNGの場合は中断
            if (CheckEntries() == false) {
                return;
            }

            // 画面入力値をパラメーターに保持
            PinNew = textPin.Text;
            PinOld = textPinOld.Text;
            CommandTitle = ToolGUICommon.PROCESS_NAME_CLIENT_PIN_CHANGE;

            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.OK);
        }

        private bool CheckEntries()
        {
            return true;
        }
    }
}
