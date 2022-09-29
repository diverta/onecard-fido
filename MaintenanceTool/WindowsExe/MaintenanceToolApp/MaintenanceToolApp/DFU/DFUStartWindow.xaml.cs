using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.DFU
{
    /// <summary>
    /// Window1.xaml の相互作用ロジック
    /// </summary>
    public partial class DFUStartWindow : Window
    {
        // 処理実行のためのプロパティー
        private readonly DFUParameter Parameter;

        public DFUStartWindow(DFUParameter param)
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

        private void TerminateWindow(bool dialogResult)
        {
            // この画面を閉じる
            DialogResult = dialogResult;
            Close();
        }

        private void DoUSBDFU()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // 転送区分を設定し、画面を閉じる
            Parameter.Transport = Transport.TRANSPORT_CDC_ACM;
            TerminateWindow(true);
        }

        private void DoBLEDFU()
        {
            // 処理前の確認
            if (DFUProcess.ConfirmDoProcess(this) == false) {
                return;
            }

            // 転送区分を設定し、画面を閉じる
            Parameter.Transport = Transport.TRANSPORT_BLE;
            TerminateWindow(true);
        }

        //
        // イベント処理部
        // 
        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }

        private void buttonBLEDFU_Click(object sender, RoutedEventArgs e)
        {
            DoBLEDFU();
        }

        private void buttonUSBDFU_Click(object sender, RoutedEventArgs e)
        {
            DoUSBDFU();
        }
    }
}
