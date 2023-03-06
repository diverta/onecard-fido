using System.ComponentModel;
using System.Windows;

namespace MaintenanceTool.OATH
{
    /// <summary>
    /// TOTPDisplayWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class TOTPDisplayWindow : Window
    {
        // パラメーターの参照を保持
        private readonly OATHParameter Parameter;

        public TOTPDisplayWindow(OATHParameter parameter)
        {
            // パラメーターの参照を保持
            Parameter = parameter;

            // 画面項目の初期化
            InitializeComponent();
        }

        public void ShowDialogWithOwner(Window ownerWindow)
        {
            // この画面を、親画面の中央にモード付きで表示
            Owner = ownerWindow;
            ownerWindow.Hide();
            ShowDialog();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            Close();
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            // 親画面を表示
            Owner.Show();
        }
    }
}
