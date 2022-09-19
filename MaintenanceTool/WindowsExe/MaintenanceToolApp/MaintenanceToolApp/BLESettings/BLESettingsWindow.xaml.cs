using System.Windows;

namespace MaintenanceToolApp.BLESettings
{
    /// <summary>
    /// BLESettingsWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class BLESettingsWindow : Window
    {
        // 処理実行のためのプロパティー
        private readonly BLESettingsParameter Parameter;

        public BLESettingsWindow(BLESettingsParameter param)
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

        //
        // イベント処理部
        // 
        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }

        private void buttonPairing_Click(object sender, RoutedEventArgs e)
        {
            // TODO: 仮の実装です。
            new PairingStartWindow(Parameter).ShowDialogWithOwner(this);
        }
    }
}
