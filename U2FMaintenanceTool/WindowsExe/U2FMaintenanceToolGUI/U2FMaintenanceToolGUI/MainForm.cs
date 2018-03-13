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
            if (app.commandAvailable) {
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
            if (checkCommandAvailable() == false) {
                return;
            }

            // ボタンに対応する処理を実行
            if (sender.Equals(button1)) {
                commandTitle = "ペアリング情報削除処理";
                ret = app.doEraseBond();

            } else if (sender.Equals(button2)) {
                commandTitle = "鍵・証明書・キーハンドル削除処理";
                ret = app.doEraseSkeyCert();

            } else if (sender.Equals(button3)) {
                commandTitle = "鍵・証明書インストール";
                ret = app.doInstallSkeyCert(textPath1.Text, textPath2.Text);

            } else if (sender.Equals(button4)) {
                commandTitle = "ヘルスチェック";
                ret = app.doHealthCheck();

            } else if (sender.Equals(button5)) {
                commandTitle = "Chrome Native Messaging有効化設定";
                ret = app.doSetupChromeNativeMessaging();
            }

            // 処理結果を画面表示し、ボタンを押下可能とする
            displayResultMessage(commandTitle, ret);
            enableButtons(true);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            // ペアリング情報削除
            doCommand(sender);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}\n{2}",
                "One Cardから鍵・証明書・キーハンドルをすべて削除します。",
                "削除後はOne CardによるU2F認証ができなくなります。",
                "削除処理を実行しますか？");

            // 鍵・証明書削除
            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (displayPromptPopup(message)) {
                doCommand(sender);
            }
        }

        private bool checkPathEntry(TextBox textBox, string errorMessage)
        {
            // 入力済みの場合はチェックOK
            if (textBox.Text.Length > 0) {
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
            if (checkPathEntry(textPath1, "鍵ファイルのパスを選択してください") == false) {
                return;
            }
            if (checkPathEntry(textPath2, "証明書ファイルのパスを選択してください") == false) {
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
            if (app.checkChromeNMSettingFileIsExist()) {
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
            if (checkSettingJsonIsExist() == false) {
                return;
            }

            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}\n{2}",
                "ChromeでBLE U2Fトークンが使用できるよう設定します。",
                "ChromeでBLE U2Fトークンを使用時、U2FMaintenanceTool.exeが、Chromeのサブプロセスとしてバックグラウンド起動されます。",
                "設定を実行しますか？");

            // Chrome Native Messaging有効化設定
            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (displayPromptPopup(message)) {
                doCommand(sender);
            }
        }

        private void buttonPath1_Click(object sender, EventArgs e)
        {
            selectFilePath(
                "秘密鍵ファイル(PEM)を選択してください",
                "秘密鍵ファイル (*.pem)|*.pem",
                textPath1);
        }

        private void buttonPath2_Click(object sender, EventArgs e)
        {
            selectFilePath(
                "証明書ファイル(CRT)を選択してください", 
                "証明書ファイル (*.crt)|*.crt", 
                textPath2);
        }

        private void selectFilePath(string title, string filter, TextBox textBox)
        {
            // ファイル選択ダイアログで選択されたパスを
            // 指定のテキストボックスにセット
            openFileDialog1.FileName = "";
            openFileDialog1.Title = title;
            openFileDialog1.Filter = filter;
            openFileDialog1.FilterIndex = 0;
            openFileDialog1.RestoreDirectory = true;
            DialogResult dr = openFileDialog1.ShowDialog();
            if (dr == DialogResult.OK) {
                textBox.Text = openFileDialog1.FileName;
            }
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
            if (success == false) {
                textBox1.AppendText(app.getProcessErrorData());
            }

            // U2F管理コマンドの実行結果を表示
            string formatted = string.Format("{0}が{1}しました。",
                message, success ? "成功" : "失敗");
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
            // TODO:後日実装
        }

        private void 自己署名証明書ファイル作成SToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // TODO:後日実装
        }

        private void doCreatePrivateKeyFile(object sender)
        {
            string filePath = createFilePath(
                    "作成する秘密鍵ファイル(PEM)名を指定してください",
                    "U2FPrivKey",
                    "秘密鍵ファイル (*.pem)|*.pem"
                    );
            doCreateFile(sender, filePath);
        }

        private bool checkOpensslAvailable()
        {
            // OpenSSLコマンドがある場合はOK
            if (app.opensslAvailable) {
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
            if (filePath.Equals(string.Empty)) {
                return;
            }

            // OpenSSLコマンドが存在しない場合は終了
            if (checkOpensslAvailable() == false) {
                return;
            }

            // ボタンを押下不可とする
            enableButtons(false);

            // ボタンに対応する処理を実行
            if (sender.Equals(鍵ファイル作成KToolStripMenuItem)) {
                commandTitle = "鍵ファイル作成処理";
                ret = app.doCreatePrivateKey(filePath);

            }
            else if (sender.Equals(証明書要求ファイル作成RToolStripMenuItem)) {
                commandTitle = "証明書要求ファイル作成処理";

            }
            else if (sender.Equals(自己署名証明書ファイル作成SToolStripMenuItem)) {
                commandTitle = "自己署名証明書ファイル作成処理";

            }

            // 処理結果を画面表示し、ボタンを押下可能とする
            displayResultMessage(commandTitle, ret);
            enableButtons(true);
        }

        private string createFilePath(string title, string fileName, string filter)
        {
            // ファイル保存ダイアログで生成されたパスを戻す
            saveFileDialog1.FileName = fileName;
            saveFileDialog1.Title = title;
            saveFileDialog1.Filter = filter;
            saveFileDialog1.FilterIndex = 0;
            saveFileDialog1.RestoreDirectory = true;
            DialogResult dr = saveFileDialog1.ShowDialog();
            if (dr != DialogResult.OK) {
                return string.Empty;
            }
            return saveFileDialog1.FileName;
        }
    }
}
