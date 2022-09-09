using System.Windows;

namespace MaintenanceToolApp.DFU
{
    /// <summary>
    /// DFUWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class DFUWindow : Window
    {
        // 処理実行のためのプロパティー
        private readonly DFUParameter Parameter;

        public DFUWindow(DFUParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
            InitializeComponent();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // パラメーターのバージョン情報を画面表示
            labelCurrentVersion.Content = Parameter.CurrentVersionInfo.FWRev;
            labelUpdateVersion.Content = Parameter.UpdateImageData.UpdateVersion;

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

        //
        // イベント処理部
        // 
        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }

        private void buttonOK_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(true);
        }
    }
}
