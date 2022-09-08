using System.Windows;

namespace MaintenanceToolApp.Common
{
    /// <summary>
    /// CommonProcessingWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class CommonProcessingWindow : Window
    {
        private static CommonProcessingWindow Instance = null!;

        private CommonProcessingWindow()
        {
            InitializeComponent();
        }

        private bool OpenFormInner(Window ownerWindow)
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

        private void NotifyTerminateInner()
        {
            // 処理進捗画面を閉じる
            Close();
        }

        //
        // 公開用メソッド
        //
        public static bool OpenForm(Window owner)
        {
            Instance = new CommonProcessingWindow();
            return Instance.OpenFormInner(owner);
        }

        public static void NotifyTerminate()
        {
            Instance.NotifyTerminateInner();
            Instance = null!;
        }
    }
}
