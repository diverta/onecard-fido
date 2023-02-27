using MaintenanceToolApp;
using System.ComponentModel;
using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceTool.OATH
{
    /// <summary>
    /// OATHWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class OATHWindow : Window
    {
        // 処理パラメーターの参照を保持
        private readonly TOTPParameter Parameter = null!;

        public OATHWindow()
        {
            // 処理パラメーターの参照を保持
            Parameter = new TOTPParameter();

            // 画面項目の初期化
            InitializeComponent();
            InitFieldValue();
        }

        public void ShowDialogWithOwner(Window ownerWindow)
        {
            // この画面を、親画面の中央にモード付きで表示
            Owner = ownerWindow;
            ownerWindow.Hide();
            ShowDialog();
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
            if (Parameter.Transport == Transport.TRANSPORT_HID) {
                if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                    return;
                }
            }

            // QRコードのスキャン画面を表示
            new ScanQRCodeWindow().ShowDialogWithOwner(this);
        }

        private void DoShowPassword()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (Parameter.Transport == Transport.TRANSPORT_HID) {
                if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                    return;
                }
            }

            // TODO: 仮の実装です。
            DialogUtil.ShowWarningMessage(this, Title, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
        }

        private void DoDeleteAccount()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (Parameter.Transport == Transport.TRANSPORT_HID) {
                if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                    return;
                }
            }

            // TODO: 仮の実装です。
            DialogUtil.ShowWarningMessage(this, Title, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
        }

        //
        // イベント処理部
        // 
        private void buttonTransportUSB_Checked(object sender, RoutedEventArgs e)
        {
            DoSetParameter(Transport.TRANSPORT_HID);
        }

        private void buttonTransportBLE_Checked(object sender, RoutedEventArgs e)
        {
            DoSetParameter(Transport.TRANSPORT_BLE);
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            // この画面を閉じる
            Close();
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            // 親画面を表示
            Owner.Show();
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
