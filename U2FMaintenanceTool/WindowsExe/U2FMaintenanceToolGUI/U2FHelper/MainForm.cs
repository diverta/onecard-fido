using System;
using System.Windows.Forms;

namespace U2FHelper
{
    public partial class MainForm : Form
    {
        private HIDProcess p = new HIDProcess();

        public MainForm()
        {
            InitializeComponent();

            // バージョン表示
            label2.Text = "Version 0.1.0";

            // U2F HIDデバイスに接続
            p.OnFormCreate(this);
        }

        private void 終了ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TerminateApplication();
        }

        private void 画面を表示ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // 画面を再表示する
            TopMost = true;
            Visible = true;
            TopMost = false;
        }

        private void ButtonQuit_Click(object sender, EventArgs e)
        {
            TerminateApplication();
        }

        private void TerminateApplication()
        {
            // U2F HIDデバイスを切断
            p.OnFormDestroy();

            // アイコンを非表示にしてアプリケーションを終了
            notifyIcon1.Visible = false;
            Application.Exit();
        }

        private void ButtonHide_Click(object sender, EventArgs e)
        {
            // 画面を非表示にする
            TopMost = false;
            Visible = false;
        }

        public void PrintText(string s)
        {
            textBox1.AppendText(s);
        }

        protected override void WndProc(ref Message m)
        {
            base.WndProc(ref m);

            if (m.Msg == WmDevicechange) {
                int wParam = m.WParam.ToInt32();
                if (wParam == DbtDevicearrival) {
                    p.OnUSBDeviceArrival();
                }
                if (wParam == DbtDeviceremovecomplete) {
                    p.OnUSBDeviceRemoveComplete();
                }
            }
        }

        // WndProc で使用する定数
        public const int WmDevicechange = 0x0219;
        public const int DbtDevicearrival = 0x8000;
        public const int DbtDeviceremovecomplete = 0x8004;
    }
}
