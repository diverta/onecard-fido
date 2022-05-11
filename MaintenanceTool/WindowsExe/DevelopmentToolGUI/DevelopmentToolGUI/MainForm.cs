using System;
using System.Diagnostics;
using System.Reflection;
using System.Windows.Forms;
using ToolGUICommon;

namespace DevelopmentToolGUI
{
    public partial class MainForm : Form
    {
        private HIDMain HIDMainRef;
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
            AppUtil.OutputLogInfo(String.Format(
                "{0}を起動しました: {1}", MaintenanceToolTitle, MaintenanceToolVersion));

            // HIDMainクラスを生成
            HIDMainRef = new HIDMain(this);

            // 画面タイトルを設定
            Text = MaintenanceToolTitle;

            // コマンドタイムアウト発生時の処理
            commandTimer = new CommandTimer(Name, 30000);
            commandTimer.CommandTimeoutEvent += CommandTimerElapsed;
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
            HIDMainRef.OnFormDestroy();
            AppUtil.OutputLogInfo(String.Format("{0}を終了しました", MaintenanceToolTitle));
        }

        public void OnPrintMessageText(string messageText)
        {
            textBox1.AppendText(messageText + "\r\n");
        }

        private bool DoCommandTimedOut(object sender, EventArgs e)
        {
            // MainForm内で終了処理を継続する
            return false;
        }

        public void OnAppMainProcessExited(bool ret)
        {
            // コマンドタイムアウト監視終了
            commandTimer.Stop();

            // 処理結果を画面表示し、ボタンを押下可能とする
            displayResultMessage(commandTitle, ret);
            enableButtons(true);
        }

        public bool CheckUSBDeviceDisconnected()
        {
            if (HIDMainRef.IsUSBDeviceDisconnected()) {
                FormUtil.ShowWarningMessage(this, MaintenanceToolTitle, AppCommon.MSG_CMDTST_PROMPT_USB_PORT_SET);
                return true;
            }
            return false;
        }

        private void enableButtons(bool enabled)
        {
            buttonFIDO.Enabled = enabled;
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

            if (commandTitle.Equals(AppCommon.PROCESS_NAME_ERASE_SKEY_CERT)) {
                // 鍵・証明書消去
                HIDMainRef.DoEraseSkeyCert();

            } else if (commandTitle.Equals(AppCommon.PROCESS_NAME_INSTALL_SKEY_CERT)) {
                // 鍵・証明書インストール
                HIDMainRef.DoInstallSkeyCert(f.KeyPath, f.CertPath);
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

            if (commandTitle.Equals(AppCommon.PROCESS_NAME_TOOL_VERSION_INFO)) {
                // バージョン情報フォームを表示
                ToolVersionForm vf = new ToolVersionForm();
                vf.ShowToolVersionDialog(
                    AppCommon.MSG_DIALOG_NAME_TOOL_VERSION_INFO,
                    MaintenanceToolTitle,
                    MaintenanceToolVersion,
                    MaintenanceToolCopyright);

            } else if (commandTitle.Equals(AppCommon.PROCESS_NAME_VIEW_LOG_FILE)) {
                // 管理ツールのログファイルを格納している
                // フォルダーを、Windowsのエクスプローラで参照
                Process.Start(AppUtil.OutputLogFileDirectoryPath());
            }
        }

        protected override void WndProc(ref Message m)
        {
            base.WndProc(ref m);

            if (m.Msg == WmDevicechange) {
                int wParam = m.WParam.ToInt32();
                if (wParam == DbtDevicearrival) {
                    HIDMainRef.OnUSBDeviceArrival();
                }
                if (wParam == DbtDeviceremovecomplete) {
                    HIDMainRef.OnUSBDeviceRemoveComplete();
                }
            }
        }

        // WndProc で使用する定数
        public const int WmDevicechange = 0x0219;
        public const int DbtDevicearrival = 0x8000;
        public const int DbtDeviceremovecomplete = 0x8004;
    }
}
