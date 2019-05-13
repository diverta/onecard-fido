﻿using System;
using System.Windows.Forms;
using MaintenanceToolCommon;

namespace MaintenanceToolGUI
{
    public partial class MainForm : Form
    {
        private AppMain app;
        private HIDMain hid;
        private string commandTitle = "";

        // 管理ツールの情報
        public const string MaintenanceToolTitle = "FIDO認証器管理ツール";
        public const string MaintenanceToolVersion = "Version 0.1.11";

        public MainForm()
        {
            InitializeComponent();
            app = new AppMain(this);
            hid = new HIDMain(this);

            // 画面タイトルを設定
            Text = MaintenanceToolTitle;
        }

        private void buttonQuit_Click(object sender, EventArgs e)
        {
            // このアプリケーションを終了する
            hid.OnFormDestroy();
            app.doExit();
        }

        public void OnPrintMessageText(string messageText)
        {
            textBox1.AppendText(messageText + "\r\n");
        }

        private void doCommand(object sender)
        {
            // ボタンを押下不可とする
            enableButtons(false);

            // ボタンに対応する処理を実行
            if (sender.Equals(button1)) {
                commandTitle = ToolGUICommon.PROCESS_NAME_PAIRING;
                DisplayStartMessage(commandTitle);
                app.doPairing();

            }
            else if (sender.Equals(button2)) {
                commandTitle = ToolGUICommon.PROCESS_NAME_ERASE_SKEY_CERT;
                DisplayStartMessage(commandTitle);
                hid.DoEraseSkeyCert();

            }
            else if (sender.Equals(button3)) {
                commandTitle = ToolGUICommon.PROCESS_NAME_INSTALL_SKEY_CERT;
                DisplayStartMessage(commandTitle);
                hid.DoInstallSkeyCert(textPath1.Text, textPath2.Text);

            }
            else if (sender.Equals(cTAPHIDINIT実行ToolStripMenuItem)) {
                commandTitle = ToolGUICommon.PROCESS_NAME_TEST_CTAPHID_INIT;
                hid.DoTestCtapHidInit();
            }
            else if (sender.Equals(DoHealthCheckToolStripMenuItem)) {
                commandTitle = ToolGUICommon.PROCESS_NAME_U2F_HEALTHCHECK;
                DisplayStartMessage(commandTitle);
                app.doHealthCheck();

            }
        }

        private void DoCommandClientPinSet(object sender, EventArgs e)
        {
            // パラメーター入力画面を表示
            SetPinParamForm f = new SetPinParamForm();
            if (f.ShowDialog() == DialogResult.Cancel) {
                // パラメーター入力画面でCancelの場合は終了
                return;
            }

            // ボタンを押下不可とする
            enableButtons(false);
            // 開始メッセージを表示
            commandTitle = f.CommandTitle;
            DisplayStartMessage(commandTitle);

            // PINコード設定
            hid.DoClientPinSet(f.PinNew, f.PinOld);
        }

        private void DoCommandCtap2Healthcheck(object sender, EventArgs e)
        {
            // パラメーター入力画面を表示
            PinCodeParamForm f = new PinCodeParamForm();
            if (f.ShowDialog() == DialogResult.Cancel) {
                // パラメーター入力画面でCancelの場合は終了
                return;
            }

            // ボタンを押下不可とする
            enableButtons(false);
            // 開始メッセージを表示
            commandTitle = ToolGUICommon.PROCESS_NAME_CTAP2_HEALTHCHECK;
            DisplayStartMessage(commandTitle);

            // CTAP2ヘルスチェック実行
            hid.DoCtap2Healthcheck(f.PinCurr);
        }

        public void OnAppMainProcessExited(bool ret)
        {
            // 処理結果を画面表示し、ボタンを押下可能とする
            displayResultMessage(commandTitle, ret);
            enableButtons(true);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            // ペアリング実行
            doCommand(sender);
        }

        private bool CheckUSBDeviceDisconnected()
        {
            if (hid.IsUSBDeviceDisconnected()) {
                MessageBox.Show(AppCommon.MSG_CMDTST_PROMPT_USB_PORT_SET, MaintenanceToolTitle);
                return true;
            }
            return false;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}",
                ToolGUICommon.MSG_ERASE_SKEY_CERT,
                ToolGUICommon.MSG_PROMPT_ERASE_SKEY_CERT);

            // 鍵・証明書削除
            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (FormUtil.DisplayPromptPopup(message))
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
            MessageBox.Show(errorMessage, MaintenanceToolTitle,
                MessageBoxButtons.OK, MessageBoxIcon.Warning);
            textBox.Focus();

            return false;
        }

        private void button3_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // ファイルパス入力チェック
            if (checkPathEntry(textPath1, ToolGUICommon.MSG_PROMPT_SELECT_PKEY_PATH) == false)
            {
                return;
            }
            if (checkPathEntry(textPath2, ToolGUICommon.MSG_PROMPT_SELECT_CRT_PATH) == false)
            {
                return;
            }

            // 鍵・証明書インストール
            doCommand(sender);
        }

        private void button4_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            DoCommandClientPinSet(sender, e);
        }

        private void buttonPath1_Click(object sender, EventArgs e)
        {
            FormUtil.selectFilePath(openFileDialog1,
                ToolGUICommon.MSG_PROMPT_SELECT_PKEY_PATH,
                ToolGUICommon.FILTER_SELECT_PEM_PATH,
                textPath1);
        }

        private void buttonPath2_Click(object sender, EventArgs e)
        {
            FormUtil.selectFilePath(openFileDialog1,
                ToolGUICommon.MSG_PROMPT_SELECT_CRT_PATH,
                ToolGUICommon.FILTER_SELECT_CRT_PATH,
                textPath2);
        }

        private void enableButtons(bool enabled)
        {
            button1.Enabled = enabled;
            button2.Enabled = enabled;
            button3.Enabled = enabled;
            button4.Enabled = enabled;
            buttonPath1.Enabled = enabled;
            buttonPath2.Enabled = enabled;
            textPath1.Enabled = enabled;
            textPath2.Enabled = enabled;
            buttonQuit.Enabled = enabled;
            menuStrip1.Enabled = enabled;
        }

        private void DisplayStartMessage(string message)
        {
            // 処理開始メッセージを表示
            string formatted = string.Format(ToolGUICommon.MSG_FORMAT_START_MESSAGE, message);
            textBox1.AppendText(formatted + "\r\n");
        }

        private void displayResultMessage(string message, bool success)
        {
            // コマンドの実行結果を表示
            string formatted = string.Format(ToolGUICommon.MSG_FORMAT_END_MESSAGE,
                message, success ? ToolGUICommon.MSG_SUCCESS : ToolGUICommon.MSG_FAILURE);
            textBox1.AppendText(formatted + "\r\n");
            MessageBox.Show(this, formatted, MaintenanceToolTitle);
        }

        private void 管理ツールについてToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // バージョン表示画面を表示
            AboutForm f = new AboutForm();
            f.SetTitleAndVersionText(MaintenanceToolTitle, MaintenanceToolVersion);
            f.ShowDialog();
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
        }

        private void toolStripMenuItem1_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // CTAP2ヘルスチェック実行
            DoCommandCtap2Healthcheck(sender, e);
        }

        private void cTAPHIDINIT実行ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // CTAPHID_INITのテストを実行
            doCommand(sender);
        }

        private void DoHealthCheckToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // U2Fヘルスチェック実行
            doCommand(sender);
        }

        protected override void WndProc(ref Message m)
        {
            base.WndProc(ref m);

            if (m.Msg == WmDevicechange) {
                int wParam = m.WParam.ToInt32();
                if (wParam == DbtDevicearrival) {
                    hid.OnUSBDeviceArrival();
                }
                if (wParam == DbtDeviceremovecomplete) {
                    hid.OnUSBDeviceRemoveComplete();
                }
            }
        }

        // WndProc で使用する定数
        public const int WmDevicechange = 0x0219;
        public const int DbtDevicearrival = 0x8000;
        public const int DbtDeviceremovecomplete = 0x8004;
    }
}
