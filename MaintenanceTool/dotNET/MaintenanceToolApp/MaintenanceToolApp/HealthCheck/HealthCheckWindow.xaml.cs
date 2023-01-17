using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.HealthCheck
{
    /// <summary>
    /// HealthCheckWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class HealthCheckWindow : Window
    {
        // 処理実行のためのプロパティー
        private readonly HealthCheckParameter Parameter;

        public HealthCheckWindow(HealthCheckParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
            InitializeComponent();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // この画面を、オーナー画面の中央にモード付きで表示
            Owner = ownerWindow;
            bool? b = ShowDialog();
            if (b == null) {
                return false;
            } else {
                return (bool)b;
            }
        }

        private void DoBLECtap2HealthCheck()
        {
            // PINコード入力画面を表示
            HealthCheckPinWindow w = new HealthCheckPinWindow(Parameter);
            if (w.ShowDialogWithOwner(this) == false) {
                return;
            }

            // 実行機能を設定し、画面を閉じる
            Parameter.Command = Command.COMMAND_BLE_CTAP2_HCHECK;
            Parameter.Transport = Transport.TRANSPORT_BLE;
            TerminateWindow(true);
        }

        private void DoBLEU2FHealthCheck()
        {
            // 実行機能を設定し、画面を閉じる
            Parameter.Command = Command.COMMAND_BLE_U2F_HCHECK;
            Parameter.Transport = Transport.TRANSPORT_BLE;
            TerminateWindow(true);
        }

        private void DoBLEPingTest()
        {
            // 実行機能を設定し、画面を閉じる
            Parameter.Command = Command.COMMAND_TEST_BLE_PING;
            Parameter.Transport = Transport.TRANSPORT_BLE;
            TerminateWindow(true);
        }

        private void DoHIDCtap2HealthCheck()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // PINコード入力画面を表示
            HealthCheckPinWindow w = new HealthCheckPinWindow(Parameter);
            if (w.ShowDialogWithOwner(this) == false) {
                return;
            }

            // 実行機能を設定し、画面を閉じる
            Parameter.Command = Command.COMMAND_HID_CTAP2_HCHECK;
            Parameter.Transport = Transport.TRANSPORT_HID;
            TerminateWindow(true);
        }

        private void DoHIDU2FHealthCheck()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // 実行機能を設定し、画面を閉じる
            Parameter.Command = Command.COMMAND_HID_U2F_HCHECK;
            Parameter.Transport = Transport.TRANSPORT_HID;
            TerminateWindow(true);
        }

        private void DoHIDPingTest()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // 実行機能を設定し、画面を閉じる
            Parameter.Command = Command.COMMAND_TEST_CTAPHID_PING;
            Parameter.Transport = Transport.TRANSPORT_HID;
            TerminateWindow(true);
        }

        private void TerminateWindow(bool dialogResult)
        {
            // この画面を閉じる
            DialogResult = dialogResult;
            Close();
        }

        //
        // イベント処理部
        // 
        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }

        private void buttonBLECtap2HealthCheck_Click(object sender, RoutedEventArgs e)
        {
            DoBLECtap2HealthCheck();
        }

        private void buttonBLEU2FHealthCheck_Click(object sender, RoutedEventArgs e)
        {
            DoBLEU2FHealthCheck();
        }

        private void buttonBLEPingTest_Click(object sender, RoutedEventArgs e)
        {
            DoBLEPingTest();
        }

        private void buttonHIDCtap2HealthCheck_Click(object sender, RoutedEventArgs e)
        {
            DoHIDCtap2HealthCheck();
        }

        private void buttonHIDU2FHealthCheck_Click(object sender, RoutedEventArgs e)
        {
            DoHIDU2FHealthCheck();
        }

        private void buttonHIDPingTest_Click(object sender, RoutedEventArgs e)
        {
            DoHIDPingTest();
        }
    }
}
