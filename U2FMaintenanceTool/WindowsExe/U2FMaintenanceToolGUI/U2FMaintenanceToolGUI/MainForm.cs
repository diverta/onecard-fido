using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
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

        private void button1_Click(object sender, EventArgs e)
        {
            // ペアリング情報削除
            enableButtons(false);
            bool ret = app.doEraseBond();
            popupResultMessage("ペアリング情報削除処理", ret);
            enableButtons(true);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            // 鍵・証明書削除
            enableButtons(false);
            bool ret = app.doEraseSkeyCert();
            popupResultMessage("鍵・証明書削除処理", ret);
            enableButtons(true);            
        }

        private void button3_Click(object sender, EventArgs e)
        {
            // 鍵・証明書インストール
            enableButtons(false);
            bool ret = app.doInstallSkeyCert(textPath1.Text, textPath2.Text);
            popupResultMessage("鍵・証明書インストール", ret);
            enableButtons(true);

        }

        private void button4_Click(object sender, EventArgs e)
        {
            // ヘルスチェック実行
            enableButtons(false);
            bool ret = app.doHealthCheck();
            popupResultMessage("ヘルスチェック", ret);
            enableButtons(true);
        }

        private void button5_Click(object sender, EventArgs e)
        {
            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}\n{2}",
                "ChromeでBLE U2Fトークンが使用できるよう設定します。",
                "ChromeでBLE U2Fトークンを使用時、U2FMaintenanceTool.exeが、Chromeのサブプロセスとしてバックグラウンド起動されます。",
                "設定を実行しますか？");

            // Chrome Native Messaging有効化設定
            // プロンプトを表示し、Yesの場合だけ処理を行う
            enableButtons(false);
            if (displayPromptPopup(message)) {
                bool ret = app.doSetupChromeNativeMessaging();
                popupResultMessage("Chrome Native Messaging有効化設定", ret);
            }
            enableButtons(true);
        }

        private void buttonPath1_Click(object sender, EventArgs e)
        {

        }

        private void buttonPath2_Click(object sender, EventArgs e)
        {

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

        private void popupResultMessage(string message, bool success)
        {
            string formatted = string.Format("{0}が{1}しました。",
                message, success ? "成功" : "失敗");
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
    }
}
