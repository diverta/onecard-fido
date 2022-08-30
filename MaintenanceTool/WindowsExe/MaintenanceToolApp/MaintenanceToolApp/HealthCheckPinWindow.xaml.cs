using System.Windows;
using ToolAppCommon;

namespace MaintenanceToolApp
{
    /// <summary>
    /// HealthCheckPinWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class HealthCheckPinWindow : Window
    {
        // 処理実行のためのプロパティー
        private readonly HealthCheckParameter Parameter;

        public HealthCheckPinWindow(HealthCheckParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
            InitializeComponent();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // PIN入力欄にフォーカスを移動
            passwordBoxPin.Focus();

            // この画面を、オーナー画面の中央にモード付きで表示
            Owner = ownerWindow;
            bool? b = ShowDialog();
            if (b == null) {
                return false;
            } else {
                return (bool)b;
            }
        }

        private void DoOK()
        {
            // TODO: 仮の実装です。

            // 入力されたPINを設定し、画面を閉じる
            Parameter.Pin = passwordBoxPin.Password;
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
        private void buttonOK_Click(object sender, RoutedEventArgs e)
        {
            DoOK();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }
    }
}
