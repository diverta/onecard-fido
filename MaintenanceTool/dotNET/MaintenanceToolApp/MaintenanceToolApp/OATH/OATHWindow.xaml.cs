using System.ComponentModel;
using System.Windows;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceTool.OATH
{
    /// <summary>
    /// OATHWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class OATHWindow : Window
    {
        public OATHWindow()
        {
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
            // TODO: トランスポート種別を設定
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
    }
}
