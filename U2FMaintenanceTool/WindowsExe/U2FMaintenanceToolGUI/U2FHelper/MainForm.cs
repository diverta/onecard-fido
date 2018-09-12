using System;
using System.Windows.Forms;

namespace U2FHelper
{
    public partial class MainForm : Form
    {
        // HIDデバイス関連
        private HIDProcess p = new HIDProcess();
        // BLEデバイス関連
        private BLEService bleService = new BLEService();

        // U2F管理コマンド関連
        private U2FMaintenanceCommand U2FCommand;
        private string U2FCommandResponse;

        public MainForm()
        {
            InitializeComponent();

            // バージョン表示
            labelVersion.Text = "Version 0.1.6";

            // イベントの登録
            p.MessageTextEvent += new HIDProcess.MessageTextEventHandler(PrintMessageText);
            p.ReceiveHIDMessageEvent += new HIDProcess.ReceiveHIDMessageEventHandler(ReceiveHIDMessage);
            bleService.MessageTextEvent += new BLEService.MessageTextEventHandler(PrintMessageText);

            // U2F HIDデバイスに接続
            //  このウィンドウのハンドルを引き渡す
            p.OnFormCreate(Handle);

            // U2F管理コマンドを初期化
            //  このフォームの参照を引き渡すことにより、
            //  U2F管理コマンドの実行が完了時、
            //  このフォームのスレッドで処理が継続されます
            U2FCommand = new U2FMaintenanceCommand(this);
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

            // U2F管理コマンドを実行し、メッセージを転送
            U2FCommand.DoXferMessage(message, length);
            U2FCommandResponse = "";
        }

        public void OnU2FCommandProcessOutputData(string outputData)
        {
            // U2F管理コマンド実行時の標準出力
            // --> base64エンコードされたレスポンスデータとして扱う
            U2FCommandResponse = outputData;
        }

        public void OnU2FCommandProcessErrorData(string errorData)
        {
            // U2F管理コマンド実行時の標準エラー出力
            // --> コマンドが出力したメッセージとして扱う
            textBox1.AppendText(errorData + "\r\n");
        }

        public void OnU2FCommandProcessExited(bool ret)
        {
            // BLEメッセージが返送されて来たら、
            // HIDデバイスにBLEメッセージを転送
            if (U2FCommandResponse.Length > 0) {
                p.XferMessage(U2FCommandResponse);
            }
            if (ret) {
                displayResultMessage(ret, AppCommon.MSG_HID_BLE_CONNECTION);

            } else {
                displayResultMessage(ret, AppCommon.MSG_U2FCOMMAND_PROCESS);
            }

            // ボタンを押下可能とする
            enableButtons(true);
        }

        private void PrintMessageText(string messageText)
        {
            // 画面のテキストエリアにメッセージを表示
            textBox1.AppendText(messageText);
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

        //
        // テストのための仮実装です
        //
        private async void button1_ClickAsync(object sender, EventArgs e)
        {
            bleService.SetMainForm(this);
            await bleService.GetU2FVersionAsync(sender, e);
        }
    }
}
