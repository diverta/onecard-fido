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
            toolBLEDFU.OnFormDestroy();
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

        private void DoCommandFIDOAttestation(object sender, EventArgs e)
        {
            // 鍵・証明書設定画面を表示
            FIDOAttestationForm f = new FIDOAttestationForm(this);
            if (f.ShowDialog() == DialogResult.Cancel) {
                // 鍵・証明書設定画面でCancelの場合は終了
                return;
            }

            // ボタンを押下不可とする
            enableButtons(false);
            // 開始メッセージを取得
            commandTitle = f.CommandTitle;
            // コマンドタイムアウト監視開始
            commandTimer.Start();

            // 鍵・証明書消去
            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_ERASE_SKEY_CERT)) {
                DisplayStartMessage(commandTitle);
                hid.DoEraseSkeyCert();
            }

            // 鍵・証明書インストール
            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_INSTALL_SKEY_CERT)) {
                DisplayStartMessage(commandTitle);
                hid.DoInstallSkeyCert(f.KeyPath, f.CertPath);
            }
        }

        private void DoCommandCtap2Healthcheck(bool bleHchk)
        {
            // パラメーター入力画面を表示
            if (PinCodeParamFormRef.OpenForm(this) == false) {
                // パラメーター入力画面でCancelの場合は終了
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

            // BLE経由のヘルスチェックはこの時点で実行
            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_BLE_CTAP2_HEALTHCHECK)) {
                // BLE CTAP2ヘルスチェック
                DoCommandCtap2Healthcheck(true);
                return;

            } else if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_BLE_U2F_HEALTHCHECK)) {
                // BLE U2Fヘルスチェック
                DoCommandU2FHealthcheck(true);
                return;
            }

            // 共有情報にヘルスチェック実行種別を設定
            ToolContext context = ToolContext.GetInstance();

            // HID経由のヘルスチェックを実行
            if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_HID_CTAP2_HEALTHCHECK)) {
                // HID CTAP2ヘルスチェック
                context.HchkType = ToolContext.HealthCheckType.CTAP2;

            } else if (commandTitle.Equals(ToolGUICommon.PROCESS_NAME_HID_U2F_HEALTHCHECK)) {
                // HID U2Fヘルスチェック
                context.HchkType = ToolContext.HealthCheckType.U2F;

            } else {
                // 該当無し
                commandTitle = "";
                OnAppMainProcessExited(false);
                return;
            }

            // コマンドタイムアウト監視開始
            commandTimer.Start();

            // ツール設定情報照会
            commandTitle = "";
            toolPreference.DoToolPreferenceParamInquiry();
        }

        public void DoResponseToolPreferenceParamInquiry()
        {
            // コマンドタイムアウト監視終了
            commandTimer.Stop();

            // 自動認証機能が有効化されている場合
            ToolContext context = ToolContext.GetInstance();
            if (context.BleScanAuthEnabled) {
                // プロンプトで表示されるメッセージ
                string message = string.Format("{0}\n\n{1}",
                    AppCommon.MSG_PROMPT_START_HCHK_BLE_AUTH,
                    AppCommon.MSG_COMMENT_START_HCHK_BLE_AUTH);

                // プロンプトを表示し、Yesの場合だけ処理を続行する
                if (FormUtil.DisplayPromptPopup(this, message) == false) {
                    OnAppMainProcessExited(true);
                    return;
                }
            }

            // ヘルスチェック実行種別に対応する処理を継続
            if (context.HchkType == ToolContext.HealthCheckType.CTAP2) {
                DoCommandCtap2Healthcheck(false);
            } else {
                DoCommandU2FHealthcheck(false);
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

        private void buttonSetPinParam_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            DoCommandClientPinSet(sender, e);
        }

        private void ButtonFIDOAttestation_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            DoCommandFIDOAttestation(sender, e);
        }

        private void enableButtons(bool enabled)
        {
            buttonBLE.Enabled = enabled;
            buttonFIDO.Enabled = enabled;
            buttonSetPinParam.Enabled = enabled;
            buttonDFU.Enabled = enabled;
            ButtonFIDOAttestation.Enabled = enabled;
            buttonSetPgpParam.Enabled = enabled;
            buttonQuit.Enabled = enabled;
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
            textBox1.AppendText(formatted + "\r\n");
            if (success) {
                AppCommon.OutputLogInfo(formatted);
                FormUtil.ShowInfoMessage(this, MaintenanceToolTitle, formatted);
            } else {
                AppCommon.OutputLogError(formatted);
                FormUtil.ShowWarningMessage(this, MaintenanceToolTitle, formatted);
            }
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
            // ヘルスチェック処理を実行
            commandTitle = ToolGUICommon.PROCESS_NAME_HID_CTAP2_HEALTHCHECK;
            DoCommandHealthCheck(sender, e);
        }

        private void DoHIDU2fTestToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // ヘルスチェック処理を実行
            commandTitle = ToolGUICommon.PROCESS_NAME_HID_U2F_HEALTHCHECK;
            DoCommandHealthCheck(sender, e);
        }

        private void DoHIDPingTestToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // PINGテストを実行
            commandTitle = ToolGUICommon.PROCESS_NAME_TEST_CTAPHID_PING;
            doCommand(sender);
        }

        private void DoHIDGetFlashInfoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // Flash ROM情報取得コマンドを実行
            commandTitle = ToolGUICommon.PROCESS_NAME_GET_FLASH_STAT;
            doCommand(sender);
        }

        private void DoHIDGetVersionInfoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (CheckUSBDeviceDisconnected()) {
                return;
            }
            // バージョン情報取得コマンドを実行
            commandTitle = ToolGUICommon.PROCESS_NAME_GET_VERSION_INFO;
            doCommand(sender);
        }

        private void DoBLECtap2TestToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // CTAP2ヘルスチェック実行
            commandTitle = ToolGUICommon.PROCESS_NAME_BLE_CTAP2_HEALTHCHECK;
            DoCommandHealthCheck(sender, e);
        }

        private void DoBLEU2fTestToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // U2Fヘルスチェック実行
            commandTitle = ToolGUICommon.PROCESS_NAME_BLE_U2F_HEALTHCHECK;
            DoCommandHealthCheck(sender, e);
        }

        private void DoBLEPingCommandToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // BLE PINGテストを実行
            commandTitle = ToolGUICommon.PROCESS_NAME_TEST_BLE_PING;
            doCommand(sender);
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

        private void ViewLogFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            // 管理ツールのログファイルを格納している
            // フォルダーを、Windowsのエクスプローラで参照
            Process.Start(AppCommon.OutputLogFileDirectoryPath());
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
