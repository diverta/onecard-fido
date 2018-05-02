using System;
using System.Windows.Forms;

namespace U2FMaintenanceToolGUI
{
    public partial class MainForm : Form
    {
        private AppMain app;

        public MainForm()
        {
            InitializeComponent();
            app = new AppMain();
        }

        private void buttonQuit_Click(object sender, EventArgs e)
        {
            // このアプリケーションを終了する
            app.doExit();
        }

        private bool checkCommandAvailable()
        {
            // U2F管理コマンドがある場合はOK
            if (app.commandAvailable)
            {
                return true;
            }

            // U2F管理コマンドがない旨の表示
            string message = "U2F管理コマンドを実行できません。";
            textBox1.AppendText(message + "\r\n");
            textBox1.AppendText(AppMain.U2FMaintenanceToolExe + "をインストールしてください。\r\n");

            // 処理結果を画面表示し、ボタンを押下可能とする
            MessageBox.Show(message, AppMain.U2FMaintenanceToolTitle, MessageBoxButtons.OK, MessageBoxIcon.Error);
            enableButtons(true);
            return false;
        }

        private void doCommand(object sender)
        {
            string commandTitle = "";
            bool ret = false;

            // ボタンを押下不可とする
            enableButtons(false);

            // U2F管理コマンドが存在しない場合は終了
            if (checkCommandAvailable() == false)
            {
                return;
            }

            // ボタンに対応する処理を実行
            if (sender.Equals(ペアリング情報消去ToolStripMenuItem))
            {
                commandTitle = AppCommon.PROCESS_NAME_ERASE_BOND;
                ret = app.doEraseBond();

            }
            else if (sender.Equals(button1))
            {
                commandTitle = AppCommon.PROCESS_NAME_PAIRING;
                ret = app.doPairing();

            }
            else if (sender.Equals(button2))
            {
                commandTitle = AppCommon.PROCESS_NAME_ERASE_SKEY_CERT;
                ret = app.doEraseSkeyCert();

            }
            else if (sender.Equals(button3))
            {
                commandTitle = AppCommon.PROCESS_NAME_INSTALL_SKEY_CERT;
                ret = app.doInstallSkeyCert(textPath1.Text, textPath2.Text);

            }
            else if (sender.Equals(button4))
            {
                commandTitle = AppCommon.PROCESS_NAME_HEALTHCHECK;
                ret = app.doHealthCheck();

            }
            else if (sender.Equals(button5))
            {
                commandTitle = AppCommon.PROCESS_NAME_SETUP_CHROME_NATIVE_MESSAGING;
                ret = app.doSetupChromeNativeMessaging();
            }

            // 処理結果を画面表示し、ボタンを押下可能とする
            displayResultMessage(commandTitle, ret);
            enableButtons(true);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            // ペアリング実行
            doCommand(sender);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}",
                AppCommon.MSG_ERASE_SKEY_CERT,
                AppCommon.MSG_PROMPT_ERASE_SKEY_CERT);

            // 鍵・証明書削除
            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (displayPromptPopup(message))
            {
                doCommand(sender);
            }
        }

        private bool checkPathEntry(TextBox textBox, string errorMessage)
        {
            // 入力済みの場合はチェックOK
            if (textBox.Text.Length > 0)
            {
                return true;
            }

            // 未入力の場合はポップアップメッセージを表示して
            // テキストボックスにフォーカスを移す
            MessageBox.Show(errorMessage, AppMain.U2FMaintenanceToolTitle,
                MessageBoxButtons.OK, MessageBoxIcon.Warning);
            textBox.Focus();

            return false;
        }

        private void button3_Click(object sender, EventArgs e)
        {
            // ファイルパス入力チェック
            if (checkPathEntry(textPath1, AppCommon.MSG_PROMPT_SELECT_PKEY_PATH) == false)
            {
                return;
            }
            if (checkPathEntry(textPath2, AppCommon.MSG_PROMPT_SELECT_CRT_PATH) == false)
            {
                return;
            }

            // 鍵・証明書インストール
            doCommand(sender);
        }

        private void button4_Click(object sender, EventArgs e)
        {
            // ヘルスチェック実行
            doCommand(sender);
        }

        private bool checkSettingJsonIsExist()
        {
            // Chrome Native Messaging設定用の
            // JSONファイルが導入済みの場合はOK
            if (app.checkChromeNMSettingFileIsExist())
            {
                return true;
            }

            // JSONファイルがない旨の表示
            string message = "Chrome Native Messagingの設定ができません。";
            textBox1.AppendText(message + "\r\n");
            textBox1.AppendText(AppMain.ChromeNMSettingFile + "をインストールしてください。\r\n");

            // メッセージをポップアップ表示
            MessageBox.Show(message, AppMain.U2FMaintenanceToolTitle, MessageBoxButtons.OK, MessageBoxIcon.Error);
            return false;
        }

        private void button5_Click(object sender, EventArgs e)
        {
            // Chrome Native Messaging設定用の
            // JSONファイルがない場合は終了
            if (checkSettingJsonIsExist() == false)
            {
                return;
            }

            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}",
                AppCommon.MSG_SETUP_CHROME,
                AppCommon.MSG_PROMPT_SETUP_CHROME);

            // Chrome Native Messaging有効化設定
            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (displayPromptPopup(message))
            {
                doCommand(sender);
            }
        }

        private void buttonPath1_Click(object sender, EventArgs e)
        {
            FormUtil.selectFilePath(openFileDialog1,
                AppCommon.MSG_PROMPT_SELECT_PKEY_PATH,
                AppCommon.FILTER_SELECT_PEM_PATH,
                textPath1);
        }

        private void buttonPath2_Click(object sender, EventArgs e)
        {
            FormUtil.selectFilePath(openFileDialog1,
                AppCommon.MSG_PROMPT_SELECT_CRT_PATH,
                AppCommon.FILTER_SELECT_CRT_PATH,
                textPath2);
        }

        private void enableButtons(bool enabled)
        {
            button1.Enabled = enabled;
            button2.Enabled = enabled;
            button3.Enabled = enabled;
            button4.Enabled = enabled;
            button5.Enabled = enabled;
            buttonPath1.Enabled = enabled;
            buttonPath2.Enabled = enabled;
            textPath1.Enabled = enabled;
            textPath2.Enabled = enabled;
            buttonQuit.Enabled = enabled;
        }

        private void displayResultMessage(string message, bool success)
        {
            // U2F管理コマンドの実行時出力内容を表示
            textBox1.AppendText(app.getProcessOutputData());
            if (success == false)
            {
                textBox1.AppendText(app.getProcessErrorData());
            }

            // U2F管理コマンドの実行結果を表示
            string formatted = string.Format(AppCommon.MSG_FORMAT_END_MESSAGE,
                message, success ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE);
            textBox1.AppendText(formatted + "\r\n");
            MessageBox.Show(formatted, AppMain.U2FMaintenanceToolTitle);
        }

        private bool displayPromptPopup(string message)
        {
            DialogResult dialogResult = MessageBox.Show(
                message, AppMain.U2FMaintenanceToolTitle,
                MessageBoxButtons.YesNo, MessageBoxIcon.Question);

            // Yesがクリックされた場合 true を戻す
            return (dialogResult == DialogResult.Yes);
        }

        private void 鍵ファイル作成KToolStripMenuItem_Click(object sender, EventArgs e)
        {
            doCreatePrivateKeyFile(sender);
        }

        private void 証明書要求ファイル作成RToolStripMenuItem_Click(object sender, EventArgs e)
        {
            doCreateCertReqFile(sender);
        }

        private void 自己署名証明書ファイル作成SToolStripMenuItem_Click(object sender, EventArgs e)
        {
            doCreateSelfCertFile(sender);
        }

        private void doCreatePrivateKeyFile(object sender)
        {
            string filePath = FormUtil.createFilePath(saveFileDialog1,
                    AppCommon.MSG_PROMPT_CREATE_PEM_PATH,
                    AppCommon.DEFAULT_PEM_NAME,
                    AppCommon.FILTER_SELECT_PEM_PATH
                    );
            doCreateFile(sender, filePath);
        }

        // パラメーター入力画面での入力項目
        public string certReqParamSubject;
        public string certReqParamKeyFile;
        public string certReqParamOutFile;

        private void doCreateCertReqFile(object sender)
        {
            // パラメーター入力画面を表示
            CertReqParamForm f = new CertReqParamForm(this);
            if (f.ShowDialog() == DialogResult.Cancel)
            {
                // パラメーター入力画面でCancelの場合は終了
                return;
            }
            doCreateFile(sender, certReqParamOutFile);
        }

        // パラメーター入力画面での入力項目
        public string selfCertParamKeyFile;
        public string selfCertParamCsrFile;
        public string selfCertParamDays;
        public string selfCertParamOutFile;

        private void doCreateSelfCertFile(object sender)
        {
            // パラメーター入力画面を表示
            SelfCertParamForm f = new SelfCertParamForm(this);
            if (f.ShowDialog() == DialogResult.Cancel)
            {
                // パラメーター入力画面でCancelの場合は終了
                return;
            }
            doCreateFile(sender, selfCertParamOutFile);
        }

        private bool checkOpensslAvailable()
        {
            // OpenSSLコマンドがある場合はOK
            if (app.opensslAvailable)
            {
                return true;
            }

            // OpenSSLコマンドがない旨の表示
            string message = "OpenSSLコマンドを実行できません。";
            textBox1.AppendText(message + "\r\n");
            textBox1.AppendText(AppMain.OpenSSLExe + "をインストールしてください。\r\n");

            // 処理結果を画面表示し、ボタンを押下可能とする
            MessageBox.Show(message, AppMain.U2FMaintenanceToolTitle, MessageBoxButtons.OK, MessageBoxIcon.Error);
            enableButtons(true);
            return false;
        }

        private void doCreateFile(object sender, string filePath)
        {
            string commandTitle = "";
            bool ret = false;

            // ファイルが生成されていない場合は終了
            if (filePath.Equals(string.Empty))
            {
                return;
            }

            // OpenSSLコマンドが存在しない場合は終了
            if (checkOpensslAvailable() == false)
            {
                return;
            }

            // ボタンを押下不可とする
            enableButtons(false);

            // ボタンに対応する処理を実行
            if (sender.Equals(鍵ファイル作成KToolStripMenuItem))
            {
                commandTitle = AppCommon.PROCESS_NAME_CREATE_KEYPAIR_PEM;
                ret = app.doCreatePrivateKey(filePath);

            }
            else if (sender.Equals(証明書要求ファイル作成RToolStripMenuItem))
            {
                commandTitle = AppCommon.PROCESS_NAME_CREATE_CERTREQ_CSR;
                ret = app.doCreateCertReq(filePath, certReqParamKeyFile, certReqParamSubject);

            }
            else if (sender.Equals(自己署名証明書ファイル作成SToolStripMenuItem))
            {
                commandTitle = AppCommon.PROCESS_NAME_CREATE_SELFCRT_CRT;
                ret = app.doCreateSelfCert(filePath, selfCertParamKeyFile, selfCertParamCsrFile, selfCertParamDays);
            }

            // 処理結果を画面表示し、ボタンを押下可能とする
            displayResultMessage(commandTitle, ret);
            enableButtons(true);
        }

        private void u2F管理ツールについてToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // バージョン表示画面を表示
            AboutForm f = new AboutForm();
            f.ShowDialog();
        }

        private void ペアリング情報消去ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}",
                AppCommon.MSG_ERASE_BONDING_INFO,
                AppCommon.MSG_PROMPT_ERASE_BONDING_INFO);

            // ペアリング情報消去
            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (displayPromptPopup(message)) {
                doCommand(sender);
            }
        }
    }
}
