using System.Windows;

namespace MaintenanceToolApp
{
    /// <summary>
    /// UtilityWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class UtilityWindow : Window
    {
        public UtilityWindow()
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
        private void buttonViewLogFile_Click(object sender, RoutedEventArgs e)
        {
            // 処理パラメーターを設定
            UtilityProcess.SetCommandTitle(AppCommon.PROCESS_NAME_VIEW_LOG_FILE);

            // 画面を閉じる
            TerminateWindow(true);
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            // 画面を閉じる
            TerminateWindow(false);
        }
    }
}
