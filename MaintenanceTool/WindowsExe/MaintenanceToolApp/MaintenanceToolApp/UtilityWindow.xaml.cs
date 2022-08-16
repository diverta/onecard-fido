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

        public void ShowDialogWithOwner(Window ownerWindow)
        {
            // ユーティリティー画面を、オーナー画面の中央にモード付きで表示
            Owner = ownerWindow;
            ShowDialog();
        }
    }
}
