using MaintenanceToolCommon;
using System;
using System.Diagnostics;
using System.Reflection;
using System.Windows.Forms;

namespace MaintenanceToolGUI
{
    public partial class MainForm : Form
    {
        private BLEMain ble;
        private HIDMain hid;
        private ToolPreference toolPreference;
        private ToolDFU toolDFU;
        private string commandTitle = "";

        // 管理ツールの情報
        public static string MaintenanceToolTitle = "";
        public static string MaintenanceToolVersion = "";
        public static string MaintenanceToolCopyright = "";

        // タイムアウト監視用タイマー
        private CommandTimer commandTimer = null;

        public MainForm()
        {
            InitializeComponent();
            MaintenanceToolTitle = GetMaintenanceToolTitle();
            MaintenanceToolVersion = String.Format("Version {0}", GetMaintenanceToolVersion());
            MaintenanceToolCopyright = GetMaintenanceToolCopyright();

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
            toolPreference.SetTitleAndVersionText();

            // DFU処理クラスを生成
            toolDFU = new ToolDFU(this, hid);
        }

        public static string GetMaintenanceToolTitle()
        {
            // タイトルを戻す
            Assembly asm = Assembly.GetExecutingAssembly();
            AssemblyTitleAttribute asmTitle =
                (AssemblyTitleAttribute)Attribute.GetCustomAttribute(
                    asm, typeof(AssemblyTitleAttribute));
            return asmTitle.Title;
        }


        private static string GetMaintenanceToolVersion()
        {
            // 製品バージョン文字列を戻す
            Assembly asm = Assembly.GetExecutingAssembly();
            System.Diagnostics.FileVersionInfo ver =
                System.Diagnostics.FileVersionInfo.GetVersionInfo(asm.Location);
            return ver.ProductVersion;
        }

        private static string GetMaintenanceToolCopyright()
        {
            // 著作権情報を戻す
            Assembly asm = Assembly.GetExecutingAssembly();
            AssemblyCopyrightAttribute copyright =
                (AssemblyCopyrightAttribute)Attribute.GetCustomAttribute(
                    asm, typeof(AssemblyCopyrightAttribute));
            return copyright.Copyright;
        }

        private void CommandTimerElapsed(object sender, EventArgs e)
        {
            // コマンドタイムアウト発生時
            // その旨を画面・ログファイルに出力
            OnPrintMessageText(AppCommon.MSG_HID_CMD_RESPONSE_TIMEOUT);
            AppCommon.OutputLogError(AppCommon.MSG_HID_CMD_RESPONSE_TIMEOUT);

            // コマンド固有の後処理を行う
            if (DoCommandTimedOut(sender, e)) {
                return;
            }

            //　コマンド終了処理を行う
            OnAppMainProcessExited(false);
        }

        private void buttonQuit_Click(object sender, EventArgs e)
        {
            // すべてのフォームを閉じる
            Application.Exit();
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            // このアプリケーションを終了する
            TerminateApplication();
        }

        private void TerminateApplication()
        {
            // このアプリケーションを終了する
            ble.OnFormDestroy();
            hid.OnFormDestroy();
            AppCommon.OutputLogInfo(String.Format("{0}を終了しました", MaintenanceToolTitle));
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

        private bool DoCommandTimedOut(object sender, EventArgs e)
        {
            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_USB_DFU)) {
                // DFU処理の場合、ToolDFU内で終了処理を行う
                //  最終的に、OnAppMainProcessExitedを経由して
                //  MainFormに異常終了が通知されます。
                toolDFU.DoCommandTimedOut();
                return true;
            }

            // MainForm内で終了処理を継続する
            return false;
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

        private void DoCommandToolPreferenceInquiry()
        {
            // ボタンを押下不可とする
            enableButtons(false);

            // コマンドタイムアウト監視開始
            commandTimer.Start();

            // ツール設定情報照会
            commandTitle = "";
            toolPreference.DoToolPreferenceParamInquiry();
        }

        public void DoResponseToolPreferenceParamInquiry(string[] fields)
        {
            // コマンドタイムアウト監視終了
            commandTimer.Stop();

            // 例外抑止
            if (fields.Length != 3) {
                OnAppMainProcessExited(true);
                return;
            }

            // 配列の先頭から、自動認証機能の有効／無効区分を取得
            bool bleScanAuthEnabled = (fields[0] == "1");
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
                FormUtil.ShowWarningMessage(MaintenanceToolTitle, AppCommon.MSG_CMDTST_PROMPT_USB_PORT_SET);
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
            FormUtil.ShowWarningMessage(MaintenanceToolTitle, errorMessage);
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
            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}",
                ToolGUICommon.MSG_INSTALL_SKEY_CERT,
                ToolGUICommon.MSG_PROMPT_INSTL_SKEY_CERT);

            // 鍵・証明書インストール
            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (FormUtil.DisplayPromptPopup(message)) {
                doCommand(sender);
            }
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
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // U2Fヘルスチェック実行
            doCommand(sender);
        }

        private void DoHIDPingTestToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // PINGテストを実行
            doCommand(sender);
        }

        private void DoHIDGetFlashInfoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // Flash ROM情報取得コマンドを実行
            doCommand(sender);
        }

        private void DoHIDGetVersionInfoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
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

        private void ViewLogFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // 管理ツールのログファイルを格納している
            // フォルダーを、Windowsのエクスプローラで参照
            Process.Start(AppCommon.OutputLogFileDirectoryPath());
        }

        private void DFUToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // DFU処理を実行
            DoCommandDFU(sender, e);
        }

        private void DFUNewToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // DFU処理を実行
            DoCommandDFU(sender, e);
        }

        //
        // DFU関連インターフェース
        //
        private void DoCommandDFU(object sender, EventArgs e)
        {
            // ボタンを押下不可とする
            enableButtons(false);

            // 処理名称を設定し、処理を開始
            commandTitle = ToolGUICommon.PROCESS_NAME_USB_DFU;
            if (sender.Equals(DFUNewToolStripMenuItem)) {
                // ファームウェア新規導入
                toolDFU.DoCommandDFUNew();
            } else {
                // ファームウェア更新
                toolDFU.DoCommandDFU();
            }
        }

        public void OnDFUStarted()
        {
            // 開始メッセージを表示
            DisplayStartMessage(commandTitle);

            // コマンドタイムアウト監視開始
            commandTimer.Start();
        }

        public void OnDFUCanceled()
        {
            // ボタンを押下可能とする
            enableButtons(true);
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
