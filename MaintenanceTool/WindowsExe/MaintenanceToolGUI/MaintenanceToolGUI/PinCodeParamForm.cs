using System;
using System.Windows.Forms;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    public partial class PinCodeParamForm : Form
    {
        // 入力されたパラメーターを保持
        public string PinCurr { get; set; }

        public PinCodeParamForm()
        {
            InitializeComponent();
        }

        protected override void OnShown(EventArgs e)
        {
            // 最初の項目にフォーカス
            textPin.Focus();
        }

        public bool OpenForm(IWin32Window owner)
        {
            // パラメーターをクリア
            InitFieldValue();

            // この画面を開く
            DialogResult = DialogResult.Cancel;
            return (ShowDialog(owner) == DialogResult.OK);
        }

        private void buttonCancel_Click(object sender, EventArgs e)
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

        private void InitFieldValue()
        {
            // 画面項目を初期値に設定
            textPin.Text = "";

            // パラメーターをクリア
            PinCurr = "";
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
            if (FormUtil.CheckEntrySize(textPin, AppCommon.PIN_CODE_SIZE_MIN, AppCommon.PIN_CODE_SIZE_MAX, MainForm.MaintenanceToolTitle, AppCommon.MSG_INVALID_FIELD_SIZE) == false) {
                return false;
            }
            if (FormUtil.CheckIsNumeric(textPin, AppCommon.MSG_INVALID_NUMBER, AppCommon.MSG_INVALID_NUMBER) == false) {
                return false;
            }
            return true;
        }
    }
}
