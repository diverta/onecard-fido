using System.Windows;
using ToolAppCommon;

namespace MaintenanceToolApp.PIV
{
    /// <summary>
    /// PIVStatusWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class PIVStatusWindow : Window
    {
        public PIVStatusWindow()
        {
            InitializeComponent();
        }

        public bool ShowDialogWithOwner(Window ownerWindow, string title, PIVParameter parameter)
        {
            // 画面項目を設定
            Title = title;
            textInfo.Text = PIVStatusWindowUtility.EditDescriptionString(parameter, CCIDProcess.GetReaderName());

            // この画面を、オーナー画面の中央にモード付きで表示
            Owner = ownerWindow;
            bool? b = ShowDialog();
            if (b == null) {
                return false;
            } else {
                return (bool)b;
            }
        }

        private void TerminateWindow()
        {
            // 画面を閉じる
            Close();
        }

        //
        // イベント処理部
        // 
        private void buttonOK_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow();
        }
    }
}
