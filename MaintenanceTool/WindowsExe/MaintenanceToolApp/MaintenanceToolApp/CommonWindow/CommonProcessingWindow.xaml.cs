using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Interop;

namespace MaintenanceToolApp.CommonWindow
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

        //
        // 閉じるボタンの無効化
        //
        [DllImport("user32.dll")]
        private static extern int GetWindowLong(IntPtr hWnd, int nIndex);

        [DllImport("user32.dll")]
        private static extern int SetWindowLong(IntPtr hWnd, int nIndex, int dwNewLong);

        const int GWL_STYLE = -16;
        const int WS_SYSMENU = 0x80000;

        protected override void OnSourceInitialized(EventArgs e)
        {
            base.OnSourceInitialized(e);
            IntPtr handle = new WindowInteropHelper(this).Handle;
            int style = GetWindowLong(handle, GWL_STYLE);
            style = style & (~WS_SYSMENU);
            SetWindowLong(handle, GWL_STYLE, style);
        }

    }
}
