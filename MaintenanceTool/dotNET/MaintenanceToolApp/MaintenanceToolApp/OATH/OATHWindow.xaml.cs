using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;
using static MaintenanceToolApp.AppDefine.Command;
using static MaintenanceToolApp.AppDefine.Transport;

namespace MaintenanceTool.OATH
{
    /// <summary>
    /// OATHWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class OATHWindow : Window
    {
        // 処理パラメーターの参照を保持
        private readonly OATHParameter Parameter = null!;

        public OATHWindow(OATHParameter parameter)
        {
            // 処理パラメーターの参照を保持
            Parameter = parameter;

            // 画面項目の初期化
            InitializeComponent();
            InitFieldValue();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // 実行機能をクリア
            Parameter.Command = COMMAND_NONE;

            // この画面を、オーナー画面の中央にモード付きで表示
            Owner = ownerWindow;
            bool? b = ShowDialog();
            if (b == null) {
                return false;
            } else {
                return (bool)b;
            }
        }

        //
        // 画面初期化処理
        //
        private void InitFieldValue()
        {
            // ラジオボタン「USB経由」を選択状態にする
            buttonTransportUSB.IsChecked = true;
        }

        private void DoSetParameter(Transport transport)
        {
            // トランスポート種別を設定
            Parameter.Transport = transport;
        }

        //
        // 各機能の処理
        //
        private void DoScanQRCode()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (Parameter.Transport == TRANSPORT_HID) {
                if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                    return;
                }
            }

            // 実行機能を設定し、画面を閉じる
            Parameter.Command = COMMAND_OATH_SCAN_QRCODE;
            TerminateWindow(true);
        }

        private void DoShowPassword()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (Parameter.Transport == TRANSPORT_HID) {
                if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                    return;
                }
            }

            // 実行機能を設定し、画面を閉じる
            Parameter.Command = COMMAND_OATH_SHOW_PASSWORD;
            TerminateWindow(true);
        }

        private void DoDeleteAccount()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (Parameter.Transport == TRANSPORT_HID) {
                if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                    return;
                }
            }

            // 実行機能を設定し、画面を閉じる
            Parameter.Command = COMMAND_OATH_DELETE_ACCOUNT;
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
        private void buttonTransportUSB_Checked(object sender, RoutedEventArgs e)
        {
            DoSetParameter(TRANSPORT_HID);
        }

        private void buttonTransportBLE_Checked(object sender, RoutedEventArgs e)
        {
            DoSetParameter(TRANSPORT_BLE);
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            // この画面を閉じる
            TerminateWindow(false);
        }

        private void buttonScanQRCode_Click(object sender, RoutedEventArgs e)
        {
            DoScanQRCode();
        }

        private void buttonShowPassword_Click(object sender, RoutedEventArgs e)
        {
            DoShowPassword();
        }

        private void buttonDeleteAccount_Click(object sender, RoutedEventArgs e)
        {
            DoDeleteAccount();
        }
    }
}
