using MaintenanceTool.OATH;
using MaintenanceToolApp.BLESettings;
using MaintenanceToolApp.DFU;
using MaintenanceToolApp.FIDOSettings;
using MaintenanceToolApp.HealthCheck;
using MaintenanceToolApp.OpenPGP;
using MaintenanceToolApp.PIV;
using MaintenanceToolApp.Utility;
using System;
using System.Reflection;
using System.Windows;
using ToolAppCommon;

namespace MaintenanceToolApp
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
        }

        protected override void OnSourceInitialized(EventArgs e)
        {
            base.OnSourceInitialized(e);

            // メイン画面のタイトルを設定
            if (IsVendorMaintenanceTool()) {
                Title = AppCommon.MSG_VENDOR_TOOL_TITLE;
            } else {
                Title = AppCommon.MSG_TOOL_TITLE;
            }

            // アプリケーション開始ログを出力
            // ログ出力を行うアプリケーション名を設定
            AppLogUtil.SetOutputLogApplName(GetApplicationName());
            AppLogUtil.OutputLogInfo(string.Format("{0}を起動しました: {1}", GetApplicationTitle(), AppUtil.GetAppVersionString()));

            // USBデバイスの脱着検知を開始
            USBDevice.StartUSBDeviceNotification(this);

            // USBデバイスの接続試行
            HIDProcess.RegisterHandlerOnConnectHIDDevice(OnConnectHIDDevice);
            HIDProcess.ConnectHIDDevice();

            // 機能クラスからのコールバックを登録
            CommandProcess.RegisterHandlerOnEnableButtonsOfMainUI(EnableButtons);
            CommandProcess.RegisterHandlerOnNotifyMessageToMainUI(AppendMessageText);
        }

        public static bool IsVendorMaintenanceTool()
        {
            return GetApplicationName().Equals("VendorMaintenanceTool");
        }

        public static string GetApplicationName()
        {
            // 製品名の文字列を戻す
            AssemblyProductAttribute attribute = Assembly.GetExecutingAssembly().GetCustomAttribute<AssemblyProductAttribute>()!;
            if (attribute == null) {
                return "ToolApp";
            } else {
                return attribute.Product;
            }
        }

        public static string GetApplicationTitle()
        {
            // アプリケーション名の文字列を戻す
            return Application.Current.MainWindow.Title;
        }

        void OnConnectHIDDevice(bool connected)
        {
            // 接続／切断検知結果をテキストボックス上に表示
            string messageText = connected ? AppCommon.MSG_HID_CONNECTED : AppCommon.MSG_HID_REMOVED;
            AppendMessageText(messageText);
        }

        void EnableButtons(bool enable)
        {
            // ボタンや入力欄の使用可能／不可制御
            buttonBLE.IsEnabled = enable;
            buttonFIDO.IsEnabled = enable;
            buttonDFU.IsEnabled = enable;
            buttonPIV.IsEnabled = enable;
            buttonOpenPGP.IsEnabled = enable;
            buttonOATH.IsEnabled = enable;
            buttonQuit.IsEnabled = enable;
            buttonHealthCheck.IsEnabled = enable;
            buttonUtility.IsEnabled = enable;
        }

        void AppendMessageText(string messageText)
        {
            // 引数の文字列を、テキストボックス上に表示し改行
            textBoxMessage.Text += messageText + "\r\n";

            // テキストボックスの現在位置を末尾に移動
            textBoxMessage.ScrollToEnd();
        }

        private void DoBLESettings()
        {
            // BLE設定画面を開き、実行コマンド種別を設定
            BLESettingsParameter param = new BLESettingsParameter();
            bool b = new BLESettingsWindow(param).ShowDialogWithOwner(this);
            if (b) {
                // BLE設定機能を実行
                new BLESettingsProcess(param).DoProcess();
            }
        }

        private void DoFIDOSettings()
        {
            // FIDO設定画面を開き、実行コマンド種別を設定
            FIDOSettingsParameter param = new FIDOSettingsParameter();
            bool b = new FIDOSettingsWindow(param).ShowDialogWithOwner(this);
            if (b) {
                // FIDO設定機能を実行
                new FIDOSettingsProcess(param).DoProcess();
            }
        }

        private void DoDFU()
        {
            // DFU開始画面を表示
            DFUParameter param = new DFUParameter();
            if (new DFUStartWindow(param).ShowDialogWithOwner(this)) {
                // ファームウェア更新画面を開き、実行を指示
                new DFUProcess(this, param).StartDFUProcess();
            }
        }

        private void DoPIV()
        {
            // PIV設定画面を開く
            new PIVWindow().ShowDialogWithOwner(this);
        }

        private void DoOpenPGP()
        {
            // OpenPGP設定画面を開く
            new OpenPGPWindow().ShowDialogWithOwner(this);
        }

        private void DoOATH()
        {
            // OATH画面を開き、実行コマンド種別を設定
            OATHParameter param = new OATHParameter();
            if (new OATHWindow(param).ShowDialogWithOwner(this)) {
                // OATH機能を実行
                new OATHWindowUtility(param).Switch(this);
            }
        }

        private void DoHealthCheck()
        {
            // ヘルスチェック実行画面を開き、実行コマンド種別を設定
            HealthCheckParameter param = new HealthCheckParameter();
            bool b = new HealthCheckWindow(param).ShowDialogWithOwner(this);
            if (b) {
                // ヘルスチェック機能を実行
                new HealthCheckProcess(param).DoProcess();
            }
        }

        private void DoUtility()
        {
            // ユーティリティー画面を開き、実行コマンド種別を設定
            UtilityParameter param = new UtilityParameter();
            bool b = new UtilityWindow(param).ShowDialogWithOwner(this);
            if (b) {
                // ユーティリティー機能を実行
                new UtilityProcess(param).DoProcess();
            }
        }

        private void TerminateWindow()
        {
            // 画面を閉じる
            Close();
        }

        //
        // イベント処理部
        // 
        private void buttonHealthCheck_Click(object sender, RoutedEventArgs e)
        {
            DoHealthCheck();
        }

        private void buttonUtility_Click(object sender, RoutedEventArgs e)
        {
            DoUtility();
        }

        private void buttonQuit_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow();
        }

        private void buttonDFU_Click(object sender, RoutedEventArgs e)
        {
            DoDFU();
        }

        private void buttonBLE_Click(object sender, RoutedEventArgs e)
        {
            DoBLESettings();
        }

        private void buttonFIDO_Click(object sender, RoutedEventArgs e)
        {
            DoFIDOSettings();
        }

        private void buttonOpenPGP_Click(object sender, RoutedEventArgs e)
        {
            DoOpenPGP();
        }

        private void buttonPIV_Click(object sender, RoutedEventArgs e)
        {
            DoPIV();
        }

        private void buttonOATH_Click(object sender, RoutedEventArgs e)
        {
            DoOATH();
        }
    }
}
