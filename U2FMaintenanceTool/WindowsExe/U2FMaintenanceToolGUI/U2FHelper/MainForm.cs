using System;
using System.Windows.Forms;

namespace U2FHelper
{
    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();

            // バージョン表示
            label2.Text = "Version 0.1.0";
        }

        private void 終了ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // アイコンを非表示にしてアプリケーションを終了
            notifyIcon1.Visible = false;
            Application.Exit();
        }

        private void 画面を表示ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // 画面を表示する
            this.TopMost = true;
            this.Visible = true;
            this.TopMost = false;
        }

        private void buttonQuit_Click(object sender, EventArgs e)
        {
            // アイコンを非表示にしてアプリケーションを終了
            notifyIcon1.Visible = false;
            Application.Exit();
        }

        private void buttonHide_Click(object sender, EventArgs e)
        {
            // 画面を非表示にする
            this.TopMost = false;
            this.Visible = false;
        }
    }
}
