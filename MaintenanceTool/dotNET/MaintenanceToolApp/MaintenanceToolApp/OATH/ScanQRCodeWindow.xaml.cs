using System.ComponentModel;
using System.Windows;

namespace MaintenanceTool.OATH
{
    /// <summary>
    /// ScanQRCodeWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class ScanQRCodeWindow : Window
    {
        public ScanQRCodeWindow()
        {
            InitializeComponent();
        }

        public void ShowDialogWithOwner(Window ownerWindow)
        {
            // この画面を、親画面の中央にモード付きで表示
            Owner = ownerWindow;
            ownerWindow.Hide();
            ShowDialog();
        }

        //
        // イベント処理部
        // 
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
