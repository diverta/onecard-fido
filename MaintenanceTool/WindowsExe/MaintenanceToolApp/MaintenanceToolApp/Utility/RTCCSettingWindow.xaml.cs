using System.Windows;

namespace MaintenanceToolApp.Utility
{
    /// <summary>
    /// RTCCSettingWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class RTCCSettingWindow : Window
    {
        public RTCCSettingWindow()
        {
            InitializeComponent();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // ユーティリティー画面を、オーナー画面の中央にモード付きで表示
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

        //
        // イベント処理部
        // 
        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }
    }
}
