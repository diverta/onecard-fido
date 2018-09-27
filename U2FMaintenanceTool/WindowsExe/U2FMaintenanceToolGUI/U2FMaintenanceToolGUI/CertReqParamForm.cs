using System;
using System.Windows.Forms;
using System.Text;

namespace U2FMaintenanceToolGUI
{
    public partial class CertReqParamForm : Form
    {
        private MainForm mainForm;

        public CertReqParamForm(MainForm f)
        {
            InitializeComponent();
            mainForm = f;
            initFieldValue();
        }

        private void initFieldValue()
        {
            // 画面項目を初期値に設定
            textPemPath.Text = "";
            textCN.Text = "";
            textOU.Text = "";
            textO.Text = "";
            textL.Text = "";
            textST.Text = "";
            textC.Text = "JP";

            // 最初の項目にフォーカス
            textPemPath.Focus();
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
                ToolGUICommon.MSG_PROMPT_CREATE_CSR_PATH,
                ToolGUICommon.DEFAULT_CSR_NAME,
                ToolGUICommon.FILTER_SELECT_CSR_PATH
                );
            if (filePath.Equals(string.Empty)) {
                return;
            }

            // 画面入力値をパラメーターに保持
            mainForm.certReqParamSubject = generateSubjectString();
            mainForm.certReqParamKeyFile = textPemPath.Text;
            mainForm.certReqParamOutFile = filePath;

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
            if (FormUtil.checkMustEntry(textPemPath, ToolGUICommon.MSG_PROMPT_SELECT_PEM_PATH) == false) {
                return false;
            }
            if (FormUtil.checkMustEntry(textCN, ToolGUICommon.MSG_PROMPT_INPUT_CN) == false) {
                return false;
            }
            if (FormUtil.checkMustEntry(textO, ToolGUICommon.MSG_PROMPT_INPUT_O) == false) {
                return false;
            }
            if (FormUtil.checkMustEntry(textL, ToolGUICommon.MSG_PROMPT_INPUT_L) == false) {
                return false;
            }
            if (FormUtil.checkMustEntry(textST, ToolGUICommon.MSG_PROMPT_INPUT_ST) == false) {
                return false;
            }
            if (FormUtil.checkMustEntry(textC, ToolGUICommon.MSG_PROMPT_INPUT_C) == false) {
                return false;
            }

            // 入力されたファイルパスが存在しない場合は終了
            if (FormUtil.checkFileExist(textPemPath, ToolGUICommon.MSG_PROMPT_EXIST_PEM_PATH) == false) {
                return false;
            }

            return true;
        }

        private string generateSubjectString()
        {
            // Subjectを編集
            // "/C=JP/ST=Tokyo/L=Shinjuku/O=Diverta inc./OU=Dev/CN=www.diverta.co.jp"
            StringBuilder sbSubject = new StringBuilder();
            sbSubject.AppendFormat("/C={0}", textC.Text);
            sbSubject.AppendFormat("/ST={0}", textST.Text);
            sbSubject.AppendFormat("/L={0}", textL.Text);
            sbSubject.AppendFormat("/O={0}", textO.Text);
            if (textOU.Text.Equals(string.Empty) == false) {
                sbSubject.AppendFormat("/OU={0}", textOU.Text);
            }
            sbSubject.AppendFormat("/CN={0}", textCN.Text);
            return sbSubject.ToString();
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
