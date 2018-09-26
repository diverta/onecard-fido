using System;
using System.Windows.Forms;

namespace U2FMaintenanceToolGUI
{
    public partial class SelfCertParamForm : Form
    {
        private MainForm mainForm;

        public SelfCertParamForm(MainForm f)
        {
            InitializeComponent();
            mainForm = f;
            initFieldValue();
        }

        private void initFieldValue()
        {
            // 画面項目を初期値に設定
            textCsrPath.Text = "";
            textPemPath.Text = "";
            textDays.Text = "365";

            // 最初の項目にフォーカス
            textCsrPath.Focus();
        }

        private void buttonCancel_Click(object sender, EventArgs e)
        {
            // 画面項目を初期化し、この画面を閉じる
            terminateWindow(DialogResult.Cancel);
        }

        private void buttonOK_Click(object sender, EventArgs e)
        {
            // 入力チェックがNGの場合は中断
            if (checkEntries() == false) {
                return;
            }

            // 出力先のファイルを選択
            string filePath = FormUtil.createFilePath(saveFileDialog1,
                ToolGUICommon.MSG_PROMPT_CREATE_CRT_PATH,
                ToolGUICommon.DEFAULT_CRT_NAME,
                ToolGUICommon.FILTER_SELECT_CRT_PATH
                );
            if (filePath.Equals(string.Empty)) {
                return;
            }

            // 画面入力値をパラメーターに保持
            mainForm.selfCertParamCsrFile = textCsrPath.Text;
            mainForm.selfCertParamKeyFile = textPemPath.Text;
            mainForm.selfCertParamDays = textDays.Text;
            mainForm.selfCertParamOutFile = filePath;

            // 画面項目を初期化し、この画面を閉じる
            terminateWindow(DialogResult.OK);
        }

        private void terminateWindow(DialogResult dialogResult)
        {
            // 画面項目を初期化し、この画面を閉じる
            initFieldValue();
            this.DialogResult = dialogResult;
            Close();
        }

        private bool checkEntries()
        {
            // 入力項目が正しく指定されていない場合は終了
            if (FormUtil.checkMustEntry(textCsrPath, ToolGUICommon.MSG_PROMPT_SELECT_CSR_PATH) == false) {
                return false;
            }
            if (FormUtil.checkMustEntry(textPemPath, ToolGUICommon.MSG_PROMPT_SELECT_PEM_PATH) == false) {
                return false;
            }
            if (FormUtil.checkMustEntry(textDays, ToolGUICommon.MSG_PROMPT_INPUT_CRT_DAYS) == false) {
                return false;
            }
            if (FormUtil.checkIsNumber(textDays, ToolGUICommon.MSG_PROMPT_INPUT_CRT_DAYS) == false) {
                return false;
            }

            // 入力されたファイルパスが存在しない場合は終了
            if (FormUtil.checkFileExist(textCsrPath, ToolGUICommon.MSG_PROMPT_EXIST_CSR_PATH) == false) {
                return false;
            }
            if (FormUtil.checkFileExist(textPemPath, ToolGUICommon.MSG_PROMPT_EXIST_PEM_PATH) == false) {
                return false;
            }

            return true;
        }

        private void buttonCsrPath_Click(object sender, EventArgs e)
        {
            FormUtil.selectFilePath(openFileDialog1,
                ToolGUICommon.MSG_PROMPT_SELECT_CSR_PATH,
                ToolGUICommon.FILTER_SELECT_CSR_PATH,
                textCsrPath);
        }

        private void buttonPemPath_Click(object sender, EventArgs e)
        {
            FormUtil.selectFilePath(openFileDialog1,
                ToolGUICommon.MSG_PROMPT_SELECT_PEM_PATH,
                ToolGUICommon.FILTER_SELECT_PEM_PATH,
                textPemPath);
        }
    }
}
