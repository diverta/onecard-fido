using System.Windows;

namespace MaintenanceToolApp
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
            // TODO: 仮の実装です。
            TerminateWindow(true);
        }

        private void DoBLEU2FHealthCheck()
        {
            // TODO: 仮の実装です。
            TerminateWindow(true);
        }

        private void DoBLEPingTest()
        {
            // TODO: 仮の実装です。
            TerminateWindow(true);
        }

        private void DoHIDCtap2HealthCheck()
        {
            // TODO: 仮の実装です。
            TerminateWindow(true);
        }

        private void DoHIDU2FHealthCheck()
        {
            // TODO: 仮の実装です。
            TerminateWindow(true);
        }

        private void DoHIDPingTest()
        {
            // TODO: 仮の実装です。
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
