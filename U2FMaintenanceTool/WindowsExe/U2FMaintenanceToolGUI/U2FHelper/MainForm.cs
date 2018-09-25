using System;
using System.Windows.Forms;

namespace U2FHelper
{
    public partial class MainForm : Form
    {
        // HIDデバイス関連
        private HIDProcess p = new HIDProcess();
        // BLEデバイス関連
        private BLEProcess bleProcess = new BLEProcess();

        public MainForm()
        {
            InitializeComponent();

            // バージョン表示
            labelVersion.Text = "Version 0.1.6";

            // イベントの登録
            p.MessageTextEvent += new HIDProcess.MessageTextEventHandler(OnPrintMessageText);
            p.ReceiveHIDMessageEvent += new HIDProcess.ReceiveHIDMessageEventHandler(ReceiveHIDMessage);
            bleProcess.MessageTextEvent += new BLEProcess.MessageTextEventHandler(OnPrintMessageText);
            bleProcess.ReceiveBLEMessageEvent += new BLEProcess.ReceiveBLEMessageEventHandler(ReceiveBLEMessage);

            // U2F HIDデバイスに接続
            //  このウィンドウのハンドルを引き渡す
            p.OnFormCreate(Handle);
        }

        private void enableButtons(bool enabled)
        {
            ButtonHide.Enabled = enabled;
            ButtonQuit.Enabled = enabled;
        }

        private void displayResultMessage(bool success, string processName)
        {
            // 処理の実行結果を表示
            string formatted = string.Format(
                AppCommon.MSG_FORMAT_END_MESSAGE,
                processName, 
                success ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE);
            textBox1.AppendText(formatted + "\r\n");
        }

        private void ReceiveHIDMessage(byte[] message, int length)
        {
            // ボタンを押下不可とする
            enableButtons(false);

            // 実行開始メッセージ
            string formatted = string.Format(
                AppCommon.MSG_FORMAT_START_MESSAGE,
                AppCommon.MSG_HID_BLE_CONNECTION);
            textBox1.AppendText(formatted + "\r\n");

            // BLE処理を実行し、メッセージを転送
            bleProcess.DoXferMessage(message, length);
        }

        private void ReceiveBLEMessage(bool ret, byte[] receivedMessage, int receivedLen)
        {
            // BLEメッセージが返送されて来たら
            // 先にBLEを切断し、
            // HIDデバイスにBLEメッセージを転送
            bleProcess.DisconnectBLE();
            p.XferBLEMessage(receivedMessage, receivedLen);

            // 終了メッセージ
            displayResultMessage(ret, AppCommon.MSG_HID_BLE_CONNECTION);

            // ボタンを押下可能とする
            enableButtons(true);
        }

        private void OnPrintMessageText(string messageText)
        {
            // 画面のテキストエリアにメッセージを表示
            textBox1.AppendText(messageText + "\r\n");
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

            // テキストボックスのエリアを一番最後に下げる
            textBox1.ScrollToCaret();
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
