using System;
using System.Diagnostics;
using System.Reflection;
using System.Windows.Forms;
using ToolGUICommon;

namespace MaintenanceToolGUI
{
    public partial class MainForm : Form
    {
        private BLEMain ble;
        private HIDMain hid;
        private ToolPGP toolPGP;
        private ToolBLEDFU toolBLEDFU;
        private ToolDFU toolDFU;
        private string commandTitle = "";

        // 管理ツールの情報
        public static string MaintenanceToolTitle = "";
        public static string MaintenanceToolVersion = "";
        public static string MaintenanceToolCopyright = "";

        // タイムアウト監視用タイマー
        private CommandTimer commandTimer = null;

        // パラメーター入力画面
        private PinCodeParamForm PinCodeParamFormRef;

        public MainForm()
        {
            InitializeComponent();
            MaintenanceToolTitle = GetMaintenanceToolTitle();
            MaintenanceToolVersion = String.Format("Version {0}", GetMaintenanceToolVersion());
            MaintenanceToolCopyright = GetMaintenanceToolCopyright();

            // アプリケーション開始ログを出力
            AppUtil.OutputLogInfo(String.Format(
                "{0}を起動しました: {1}", MaintenanceToolTitle, MaintenanceToolVersion));

            ble = new BLEMain(this);
            hid = new HIDMain(this);

            // 画面タイトルを設定
            Text = MaintenanceToolTitle;

            // コマンドタイムアウト発生時の処理
            commandTimer = new CommandTimer(Name, 30000);
            commandTimer.CommandTimeoutEvent += CommandTimerElapsed;

            // OpenPGP機能設定画面を生成
            toolPGP = new ToolPGP(this, hid);

            // パラメーター入力画面を生成
            PinCodeParamFormRef = new PinCodeParamForm();

            // DFU処理クラスを生成
            toolBLEDFU = new ToolBLEDFU(this, ble);
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

        private void MainForm_Shown(object sender, EventArgs e)
        {
            // 終了ボタンにフォーカス
            buttonQuit.Focus();
        }

        private void CommandTimerElapsed(object sender, EventArgs e)
        {
            // コマンドタイムアウト発生時
            // その旨を画面・ログファイルに出力
            OnPrintMessageText(AppUtil.MSG_HID_CMD_RESPONSE_TIMEOUT);
            AppUtil.OutputLogError(AppUtil.MSG_HID_CMD_RESPONSE_TIMEOUT);

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
            toolBLEDFU.OnFormDestroy();
            AppUtil.OutputLogInfo(String.Format("{0}を終了しました", MaintenanceToolTitle));
        }

        public void OnPrintMessageText(string messageText)
        {
            textBox1.AppendText(messageText + "\r\n");
        }

        private void doCommand(object sender)
        {
            // ボタンを押下不可とする
            enableButtons(false);

            // コマンドタイムアウト監視開始
            commandTimer.Start();

            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_TEST_CTAPHID_PING)) {
                // CTAPHID_INIT --> CTAPHID_PING の順に実行する
                DisplayStartMessage(commandTitle);
                hid.DoTestCtapHidPing();

            } else if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_GET_FLASH_STAT)) {
                DisplayStartMessage(commandTitle);
                hid.DoGetFlashStat();

            } else if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_GET_VERSION_INFO)) {
                DisplayStartMessage(commandTitle);
                hid.DoGetVersionInfo();

            } else if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_ERASE_BONDS)) {
                DisplayStartMessage(commandTitle);
                hid.DoEraseBonds();

            } else if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_TEST_BLE_PING)) {
                // BLE経由でPINGコマンドを実行する
                DisplayStartMessage(commandTitle);
                ble.DoTestBLEPing();

            } else {
                // エラーメッセージを画面表示し、ボタンを押下可能とする
                FormUtil.ShowWarningMessage(this, MaintenanceToolTitle, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
                commandTitle = "";
                OnAppMainProcessExited(false);
            }
        }

        private bool DoCommandTimedOut(object sender, EventArgs e)
        {
            // DFU処理の場合、ToolDFU内で終了処理を行う
            //  最終的に、OnAppMainProcessExitedを経由して
            //  MainFormに異常終了が通知されます。
            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_USB_DFU)) {
                toolDFU.DoCommandTimedOut();
                return true;
            }

            // MainForm内で終了処理を継続する
            return false;
        }

        private void DoCommandCtap2Healthcheck(bool bleHchk)
        {
            // パラメーター入力画面を表示
            if (PinCodeParamFormRef.OpenForm(this) == false) {
                // パラメーター入力画面でCancelの場合は終了
                commandTitle = "";
                OnAppMainProcessExited(true);
                return;
            }

            // 入力されたPINコードを取得
            string pin = PinCodeParamFormRef.PinCurr;

            // コマンドタイムアウト監視開始
            commandTimer.Start();

            // 開始メッセージを表示し、CTAP2ヘルスチェック実行
            if (bleHchk) {
                DisplayStartMessage(commandTitle);
                ble.DoCtap2Healthcheck(pin);
            } else {
                DisplayStartMessage(commandTitle);
                hid.DoCtap2Healthcheck(pin);
            }
        }

        private void DoCommandU2FHealthcheck(bool bleHchk)
        {
            // コマンドタイムアウト監視開始
            commandTimer.Start();

            // 開始メッセージを表示し、U2Fヘルスチェック実行
            if (bleHchk) {
                DisplayStartMessage(commandTitle);
                ble.DoU2FHealthCheck();
            } else {
                DisplayStartMessage(commandTitle);
                hid.DoU2FHealthCheck();
            }
        }

        private void DoCommandHealthCheck(object sender, EventArgs e)
        {
            // ボタンを押下不可とする
            enableButtons(false);

            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_BLE_CTAP2_HEALTHCHECK)) {
                // BLE CTAP2ヘルスチェック
                DoCommandCtap2Healthcheck(true);

            } else if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_BLE_U2F_HEALTHCHECK)) {
                // BLE U2Fヘルスチェック
                DoCommandU2FHealthcheck(true);

            } else if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_HID_CTAP2_HEALTHCHECK)) {
                // HID CTAP2ヘルスチェック
                DoCommandCtap2Healthcheck(false);

            } else if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_HID_U2F_HEALTHCHECK)) {
                // HID U2Fヘルスチェック
                DoCommandU2FHealthcheck(false);

            } else {
                // 該当無し
                commandTitle = "";
                OnAppMainProcessExited(false);
                return;
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
        }

        public bool CheckUSBDeviceDisconnected()
        {
            if (hid.IsUSBDeviceDisconnected()) {
                FormUtil.ShowWarningMessage(this, MaintenanceToolTitle, AppCommon.MSG_CMDTST_PROMPT_USB_PORT_SET);
                return true;
            }
            return false;
        }

        private void enableButtons(bool enabled)
        {
            buttonBLE.Enabled = enabled;
            buttonFIDO.Enabled = enabled;
            buttonDFU.Enabled = enabled;
            buttonSetPgpParam.Enabled = enabled;
            buttonHealthCheck.Enabled = enabled;
            buttonUtility.Enabled = enabled;
            buttonQuit.Enabled = enabled;
        }

        private void DisplayStartMessage(string message)
        {
            // 処理開始メッセージを表示
            string formatted = string.Format(AppCommon.MSG_FORMAT_START_MESSAGE, message);
            textBox1.AppendText(formatted + "\r\n");
            // ログファイルにも出力
            AppUtil.OutputLogInfo(formatted);
        }

        private void displayResultMessage(string message, bool success)
        {
            // コマンド名が設定されていない場合は終了
            if (message == "") {
                return;
            }
            // コマンドの実行結果をログファイルに出力後、
            // 画面およびメッセージボックスダイアログに表示
            string formatted = string.Format(AppCommon.MSG_FORMAT_END_MESSAGE,
                message, success ? AppCommon.MSG_SUCCESS : AppCommon.MSG_FAILURE);
            textBox1.AppendText(formatted + "\r\n");
            if (success) {
                AppUtil.OutputLogInfo(formatted);
                FormUtil.ShowInfoMessage(this, MaintenanceToolTitle, formatted);
            } else {
                AppUtil.OutputLogError(formatted);
                FormUtil.ShowWarningMessage(this, MaintenanceToolTitle, formatted);
            }
        }

        private void buttonSetPgpParam_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // OpenPGP機能設定画面を表示
            toolPGP.ShowDialog();
        }

        //
        // BLE設定関連インターフェース
        //
        private void buttonBLE_Click(object sender, EventArgs e)
        {
            // BLE設定画面を表示
            BLEForm f = new BLEForm(this);
            if (f.ShowDialog() == DialogResult.Cancel) {
                // BLE設定画面でCancelの場合は終了
                return;
            }

            // ボタンを押下不可とする
            enableButtons(false);
            // 開始メッセージを取得
            commandTitle = f.CommandTitle;

            // ペアリング実行
            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_PAIRING)) {
                DisplayStartMessage(commandTitle);
                ble.doPairing(f.GetPasskey());
            }

            // ペアリング解除
            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_ERASE_BONDS)) {
                // ペアリング情報削除コマンドを実行
                doCommand(sender);
            }
        }

        //
        // FIDO設定関連インターフェース
        //
        private void buttonFIDO_Click(object sender, EventArgs e)
        {
            // FIDO設定画面を表示
            FIDOForm f = new FIDOForm(this);
            if (f.ShowDialog() == DialogResult.Cancel) {
                // FIDO設定画面でCancelの場合は終了
                return;
            }

            // ボタンを押下不可とする
            enableButtons(false);
            // 開始メッセージを表示
            commandTitle = f.CommandTitle;
            DisplayStartMessage(commandTitle);
            // コマンドタイムアウト監視開始
            commandTimer.Start();

            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_AUTH_RESET)) {
                // FIDO認証情報の消去
                hid.DoAuthReset();

            } else if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_CLIENT_PIN_SET) || 
                commandTitle.Equals(ToolGUICommon.PROCESS_NAME_CLIENT_PIN_CHANGE)) {
                // PIN設定
                hid.DoClientPinSet(f.PinNew, f.PinOld);

            } else if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_ERASE_SKEY_CERT)) {
                // 鍵・証明書消去
                hid.DoEraseSkeyCert();

            } else if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_INSTALL_SKEY_CERT)) {
                // 鍵・証明書インストール
                hid.DoInstallSkeyCert(f.KeyPath, f.CertPath);
            }
        }

        //
        // ヘルスチェック関連インターフェース
        //
        private void buttonHealthCheck_Click(object sender, EventArgs e)
        {
            // ヘルスチェック実行画面を表示
            HealthCheckForm f = new HealthCheckForm(this);
            if (f.ShowDialog() == DialogResult.Cancel) {
                // ヘルスチェック実行画面でCancelの場合は終了
                return;
            }

            // 開始メッセージを取得
            commandTitle = f.CommandTitle;

            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_TEST_BLE_PING) ||
                commandTitle.Equals(ToolGUICommon.PROCESS_NAME_TEST_CTAPHID_PING)) {
                // PINGコマンドを実行
                doCommand(sender);

            } else if (
                commandTitle.Equals(ToolGUICommon.PROCESS_NAME_BLE_CTAP2_HEALTHCHECK) ||
                commandTitle.Equals(ToolGUICommon.PROCESS_NAME_HID_CTAP2_HEALTHCHECK) ||
                commandTitle.Equals(ToolGUICommon.PROCESS_NAME_BLE_U2F_HEALTHCHECK) ||
                commandTitle.Equals(ToolGUICommon.PROCESS_NAME_HID_U2F_HEALTHCHECK)) {
                // ヘルスチェックコマンドを実行
                DoCommandHealthCheck(sender, e);
            }
        }

        //
        // ユーティリティー関連インターフェース
        //
        private void buttonUtility_Click(object sender, EventArgs e)
        {
            // ユーティリティー画面を表示
            UtilityForm f = new UtilityForm(this);
            if (f.ShowDialog() == DialogResult.Cancel) {
                // ユーティリティー画面でCancelの場合は終了
                return;
            }

            // 開始メッセージを取得
            commandTitle = f.CommandTitle;

            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_GET_FLASH_STAT) ||
                commandTitle.Equals(ToolGUICommon.PROCESS_NAME_GET_VERSION_INFO)) {
                // HIDインターフェース経由でコマンドを実行
                doCommand(sender);

            } else if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_TOOL_VERSION_INFO)) {
                // バージョン情報フォームを表示
                ToolVersionForm vf = new ToolVersionForm();
                vf.ShowToolVersionDialog(
                    MaintenanceToolTitle, 
                    MaintenanceToolVersion, 
                    MaintenanceToolCopyright);

            } else if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_VIEW_LOG_FILE)) {
                // 管理ツールのログファイルを格納している
                // フォルダーを、Windowsのエクスプローラで参照
                Process.Start(AppUtil.OutputLogFileDirectoryPath());
            }
        }

        //
        // DFU関連インターフェース
        //
        private void buttonDFU_Click(object sender, EventArgs e)
        {
            // ファームウェア更新画面を表示
            DFUForm f = new DFUForm(this);
            if (f.ShowDialog() == DialogResult.Cancel) {
                // ファームウェア更新画面でCancelの場合は終了
                return;
            }

            // ボタンを押下不可とする
            enableButtons(false);
            // 開始メッセージを取得
            commandTitle = f.CommandTitle;

            // ファームウェア更新
            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_BLE_DFU)) {
                toolBLEDFU.DoCommandBLEDFU();
            }
            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_USB_DFU)) {
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

        public void OnBLEDFUStarted()
        {
            // 開始メッセージを表示
            DisplayStartMessage(commandTitle);
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
