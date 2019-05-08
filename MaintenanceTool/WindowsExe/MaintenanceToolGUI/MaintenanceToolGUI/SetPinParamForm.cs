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

        protected override void OnShown(EventArgs e)
        {
            // 最初の項目にフォーカス
            textPin.Focus();
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
        }

        private void buttonSetPin_Click(object sender, EventArgs e)
        {
            // 入力チェックがNGの場合は中断
            if (CheckEntries(false) == false) {
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
            if (CheckEntries(true) == false) {
                return;
            }

            // 画面入力値をパラメーターに保持
            PinNew = textPin.Text;
            PinOld = textPinOld.Text;
            CommandTitle = ToolGUICommon.PROCESS_NAME_CLIENT_PIN_CHANGE;

            // 画面項目を初期化し、この画面を閉じる
            TerminateWindow(DialogResult.OK);
        }

        private bool CheckEntries(bool change)
        {
            // 長さチェック
            if (FormUtil.checkEntrySize(textPin, ToolGUICommon.PIN_CODE_SIZE_MIN, ToolGUICommon.PIN_CODE_SIZE_MAX, ToolGUICommon.MSG_INVALID_FIELD_SIZE) == false) {
                return false;
            }
            if (FormUtil.checkEntrySize(textPinConfirm, ToolGUICommon.PIN_CODE_SIZE_MIN, ToolGUICommon.PIN_CODE_SIZE_MAX, ToolGUICommon.MSG_INVALID_FIELD_SIZE) == false) {
                return false;
            }
            if (FormUtil.checkIsNumeric(textPin, ToolGUICommon.MSG_INVALID_NUMBER) == false) {
                return false;
            }
            if (FormUtil.checkIsNumeric(textPinConfirm, ToolGUICommon.MSG_INVALID_NUMBER) == false) {
                return false;
            }

            // 変更ボタンがクリックされた場合は、変更前PINコードの入力チェックを実行
            if (change) {
                if (FormUtil.checkEntrySize(textPinOld, ToolGUICommon.PIN_CODE_SIZE_MIN, ToolGUICommon.PIN_CODE_SIZE_MAX, ToolGUICommon.MSG_INVALID_FIELD_SIZE) == false) {
                    return false;
                }
                if (FormUtil.checkIsNumeric(textPinOld, ToolGUICommon.MSG_INVALID_NUMBER) == false) {
                    return false;
                }
            }

            // 確認用PINコードのチェック
            if (FormUtil.compareEntry(textPinConfirm, textPin, ToolGUICommon.MSG_PROMPT_INPUT_PIN_CONFIRM_CRCT) == false) {
                return false;
            }

            return true;
        }

        private void buttonClearPin_Click(object sender, EventArgs e)
        {

        }
    }
}
