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
    }
}
