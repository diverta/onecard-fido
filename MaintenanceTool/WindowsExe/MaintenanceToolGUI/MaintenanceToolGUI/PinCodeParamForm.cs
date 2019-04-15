using System;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class PinCodeParamForm : Form
    {
        // 入力されたパラメーターを保持
        public string PinCurr = "";

        public PinCodeParamForm()
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

            // 最初の項目にフォーカス
            textPin.Focus();
        }

        private void buttonSetPin_Click(object sender, EventArgs e)
        {
            // 入力チェックがNGの場合は中断
            if (CheckEntries(false) == false) {
                return;
            }

            // 画面入力値をパラメーターに保持
            PinCurr = textPin.Text;

            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.OK);
        }

        private bool CheckEntries(bool change)
        {
            // 長さチェックと数字入力チェック
            if (FormUtil.checkEntrySize(textPin, ToolGUICommon.PIN_CODE_SIZE_MIN, ToolGUICommon.PIN_CODE_SIZE_MAX, ToolGUICommon.MSG_INVALID_FIELD_SIZE) == false) {
                return false;
            }
            if (FormUtil.checkIsNumeric(textPin, ToolGUICommon.MSG_INVALID_NUMBER) == false) {
                return false;
            }
            return true;
        }
    }
}
