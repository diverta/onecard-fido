using System.Windows;
using ToolAppCommon;

namespace MaintenanceToolApp
{
    /// <summary>
    /// ToolVersionWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class ToolVersionWindow : Window
    {
        public ToolVersionWindow()
        {
            InitializeComponent();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // ツールタイトル／バージョン／著作権表示
            Title = AppCommon.MSG_LABEL_NAME_TOOL_VERSION_INFO;
            labelToolName.Content = AppCommon.MSG_TOOL_TITLE;
            labelVersion.Content = AppUtil.GetAppVersionString();
            labelCopyrite.Content = AppUtil.GetAppCopyrightString();

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
            // この画面を閉じる
            Close();
        }

        //
        // イベント処理部
        // 
        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow();
        }
    }
}
