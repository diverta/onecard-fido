using System;
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

            // アプリケーション開始ログを出力
            AppLogUtil.SetOutputLogApplName("MaintenanceToolApp");
            AppLogUtil.OutputLogInfo(string.Format("{0}を起動しました: {1}", AppCommon.MSG_TOOL_TITLE, AppUtil.GetAppVersionString()));

            // USBデバイスの脱着検知を開始
            USBDevice.StartUSBDeviceNotification(this);

            // USBデバイスの接続試行
            HIDProcess.RegisterHandlerOnConnectHIDDevice(OnConnectHIDDevice);
            HIDProcess.ConnectHIDDevice();

            // 機能クラスからのコールバックを登録
            UtilityProcess.RegisterHandlerOnNotifyMessageToMainUI(AppendMessageText);
        }

        void OnConnectHIDDevice(bool connected)
        {
            // 接続／切断検知結果をテキストボックス上に表示
            string messageText = connected ? AppCommon.MSG_HID_CONNECTED : AppCommon.MSG_HID_REMOVED;
            AppendMessageText(messageText);
        }

        void AppendMessageText(string messageText)
        {
            if (DataContext is MainWindowViewModel model) {
                // 引数の文字列を、テキストボックス上に表示し改行
                model.MessageText += messageText + "\r\n";

                // テキストボックスの現在位置を末尾に移動
                textBox1.ScrollToEnd();
            }
        }

        private void DoUtility()
        {
            // ユーティリティー画面を開き、実行コマンド種別を設定
            UtilityWindow w = new UtilityWindow();
            bool b = w.ShowDialogWithOwner(this);
            if (b) {
                // ユーティリティー機能を実行
                UtilityProcess.DoProcess();
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
        private void buttonUtility_Click(object sender, RoutedEventArgs e)
        {
            DoUtility();
        }

        private void buttonQuit_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow();
        }
    }
}
