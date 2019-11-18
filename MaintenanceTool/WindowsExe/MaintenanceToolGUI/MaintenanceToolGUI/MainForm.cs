using System;
using System.Windows.Forms;
using MaintenanceToolCommon;

namespace MaintenanceToolGUI
{
    public partial class MainForm : Form
    {
        private BLEMain ble;
        private HIDMain hid;
        private ToolPreference toolPreference;
        private string commandTitle = "";

        // 管理ツールの情報
        public const string MaintenanceToolTitle = "FIDO認証器管理ツール";
        public const string MaintenanceToolVersion = "Version 0.1.20";

        // タイムアウト監視用タイマー
        private CommandTimer commandTimer = null;

        public MainForm()
        {
            InitializeComponent();

            // アプリケーション開始ログを出力
            AppCommon.OutputLogInfo(String.Format(
                "{0}を起動しました: {1}", MaintenanceToolTitle, MaintenanceToolVersion));

            ble = new BLEMain(this);
            hid = new HIDMain(this);

            // 画面タイトルを設定
            Text = MaintenanceToolTitle;

            // コマンドタイムアウト発生時の処理
            commandTimer = new CommandTimer(Name, 30000);
            commandTimer.CommandTimeoutEvent += CommandTimerElapsed;

            // ツール設定画面を生成
            // タイトル、バージョンを引き渡し
            toolPreference = new ToolPreference(this, hid);
            toolPreference.SetTitleAndVersionText(MaintenanceToolTitle, MaintenanceToolVersion);
        }

        private void CommandTimerElapsed(object sender, EventArgs e)
        {
            // コマンドタイムアウト発生時は、コマンド終了処理を行う
            OnPrintMessageText(AppCommon.MSG_HID_CMD_RESPONSE_TIMEOUT);
            AppCommon.OutputLogError(AppCommon.MSG_HID_CMD_RESPONSE_TIMEOUT);
            OnAppMainProcessExited(false);
        }

        private void buttonQuit_Click(object sender, EventArgs e)
        {
            // このアプリケーションを終了する
            DisconnectBLE();
            hid.OnFormDestroy();
            AppCommon.OutputLogInfo(String.Format("{0}を終了しました", MaintenanceToolTitle));
            Application.Exit();
        }

        public void DisconnectBLE()
        {
            // 接続ずみの場合はBLEデバイスを切断
            ble.DisconnectBLE();
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
                ble.doPairing();
                return;
            }

            // コマンドタイムアウト監視開始
            commandTimer.Start();

            if (sender.Equals(button2)) {
                commandTitle = ToolGUICommon.PROCESS_NAME_ERASE_SKEY_CERT;
                DisplayStartMessage(commandTitle);
                hid.DoEraseSkeyCert();

            }
            else if (sender.Equals(button3)) {
                commandTitle = ToolGUICommon.PROCESS_NAME_INSTALL_SKEY_CERT;
                DisplayStartMessage(commandTitle);
                hid.DoInstallSkeyCert(textPath1.Text, textPath2.Text);

            } else if (sender.Equals(DoHIDU2fTestToolStripMenuItem)) {
                commandTitle = ToolGUICommon.PROCESS_NAME_HID_U2F_HEALTHCHECK;
                DisplayStartMessage(commandTitle);
                hid.DoU2FHealthCheck();

            } else if (sender.Equals(DoHIDPingTestToolStripMenuItem)) {
                // CTAPHID_INIT --> CTAPHID_PING の順に実行する
                commandTitle = ToolGUICommon.PROCESS_NAME_TEST_CTAPHID_PING;
                DisplayStartMessage(commandTitle);
                hid.DoTestCtapHidPing();
            }
            else if (sender.Equals(DoHIDGetFlashInfoToolStripMenuItem)) {
                commandTitle = ToolGUICommon.PROCESS_NAME_GET_FLASH_STAT;
                DisplayStartMessage(commandTitle);
                hid.DoGetFlashStat();
            }
            else if (sender.Equals(DoHIDGetVersionInfoToolStripMenuItem)) {
                commandTitle = ToolGUICommon.PROCESS_NAME_GET_VERSION_INFO;
                DisplayStartMessage(commandTitle);
                hid.DoGetVersionInfo();
            } 
            else if (sender.Equals(DoBLEU2fTestToolStripMenuItem)) {
                commandTitle = ToolGUICommon.PROCESS_NAME_BLE_U2F_HEALTHCHECK;
                DisplayStartMessage(commandTitle);
                ble.DoU2FHealthCheck();
            }
            else if (sender.Equals(DoBLEPingTestToolStripMenuItem)) {
                // BLE経由でPINGコマンドを実行する
                commandTitle = ToolGUICommon.PROCESS_NAME_TEST_BLE_PING;
                DisplayStartMessage(commandTitle);
                ble.DoTestBLEPing();
            }
            else {
                // エラーメッセージを画面表示し、ボタンを押下可能とする
                MessageBox.Show(AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED, MaintenanceToolTitle);
                commandTitle = "";
                OnAppMainProcessExited(false);
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

            // コマンドタイムアウト監視開始
            commandTimer.Start();

            if (f.PinNew == "" && f.PinOld == "") {
                // パラメーター画面でPINが指定されなかった場合はPIN解除実行と判断
                hid.DoAuthReset();

            } else {
                // PINコード設定
                hid.DoClientPinSet(f.PinNew, f.PinOld);
            }
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

            // コマンドタイムアウト監視開始
            commandTimer.Start();

            // 開始メッセージを表示し、CTAP2ヘルスチェック実行
            if (sender.Equals(DoBLECtap2TestToolStripMenuItem)) {
                commandTitle = ToolGUICommon.PROCESS_NAME_BLE_CTAP2_HEALTHCHECK;
                DisplayStartMessage(commandTitle);
                ble.DoCtap2Healthcheck(f.PinCurr);
            } else {
                commandTitle = ToolGUICommon.PROCESS_NAME_HID_CTAP2_HEALTHCHECK;
                DisplayStartMessage(commandTitle);
                hid.DoCtap2Healthcheck(f.PinCurr);
            }
        }

        public void OnAppMainProcessExited(bool ret)
        {
            // コマンドタイムアウト監視終了
            commandTimer.Stop();

            // 処理結果を画面表示し、ボタンを押下可能とする
            displayResultMessage(commandTitle, ret);
            enableButtons(true);
        }

        public void OnBLEConnectionDisabled()
        {
            // BLE接続失敗時等のエラー発生時は
            // 致命的なエラーとなるため、BLE機能のメニューを使用不可にし、
            // アプリケーションを再起動させる必要がある旨のメッセージを表示
            OnAppMainProcessExited(false);
            OnPrintMessageText(AppCommon.MSG_BLE_ERR_CONN_DISABLED);
            OnPrintMessageText(AppCommon.MSG_BLE_ERR_CONN_DISABLED_SUB1);
            bLEToolStripMenuItem.Enabled = false;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            // ペアリング実行
            doCommand(sender);
        }

        public bool CheckUSBDeviceDisconnected()
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
            // ログファイルにも出力
            AppCommon.OutputLogInfo(formatted);
        }

        private void displayResultMessage(string message, bool success)
        {
            // コマンド名が設定されていない場合は終了
            if (message == "") {
                return;
            }
            // コマンドの実行結果をログファイルに出力後、
            // 画面およびメッセージボックスダイアログに表示
            string formatted = string.Format(ToolGUICommon.MSG_FORMAT_END_MESSAGE,
                message, success ? ToolGUICommon.MSG_SUCCESS : ToolGUICommon.MSG_FAILURE);
            if (success) {
                AppCommon.OutputLogInfo(formatted);
            } else {
                AppCommon.OutputLogError(formatted);
            }
            textBox1.AppendText(formatted + "\r\n");
            MessageBox.Show(this, formatted, MaintenanceToolTitle);
        }

        private void ToolPreferenceStripMenuItem_Click(object sender, EventArgs e)
        {
            // ツール設定画面を表示
            toolPreference.ShowDialog();
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
        }

        private void DoHIDCtap2TestToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // CTAP2ヘルスチェック実行
            DoCommandCtap2Healthcheck(sender, e);
        }

        private void DoHIDU2fTestToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // U2Fヘルスチェック実行
            doCommand(sender);
        }

        private void DoHIDPingTestToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // PINGテストを実行
            doCommand(sender);
        }

        private void DoHIDGetFlashInfoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // Flash ROM情報取得コマンドを実行
            doCommand(sender);
        }

        private void DoHIDGetVersionInfoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // バージョン情報取得コマンドを実行
            doCommand(sender);
        }

        private void DoBLECtap2TestToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // CTAP2ヘルスチェック実行
            DoCommandCtap2Healthcheck(sender, e);
        }

        private void DoBLEU2fTestToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // U2Fヘルスチェック実行
            doCommand(sender);
        }

        private void DoBLEPingCommandToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // BLE PINGテストを実行
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
