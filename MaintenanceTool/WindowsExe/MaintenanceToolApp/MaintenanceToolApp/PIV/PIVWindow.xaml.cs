using System.Windows;

namespace MaintenanceToolApp.PIV
{
    /// <summary>
    /// PIVWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class PIVWindow : Window
    {
        public PIVWindow()
        {
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
        private void buttonClose_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }

        private void buttonPkeyFilePath1_Click(object sender, RoutedEventArgs e)
        {
            FileDialogUtil.SelectFilePath(this, AppCommon.MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH, textPkeyFilePath1, AppCommon.MSG_FILTER_SELECT_PIV_PKEY_PEM_PATH);
        }

        private void buttonCertFilePath1_Click(object sender, RoutedEventArgs e)
        {
            FileDialogUtil.SelectFilePath(this, AppCommon.MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH, textCertFilePath1, AppCommon.MSG_FILTER_SELECT_PIV_CERT_PEM_PATH);
        }

        private void buttonPkeyFilePath2_Click(object sender, RoutedEventArgs e)
        {
            FileDialogUtil.SelectFilePath(this, AppCommon.MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH, textPkeyFilePath2, AppCommon.MSG_FILTER_SELECT_PIV_PKEY_PEM_PATH);
        }

        private void buttonCertFilePath2_Click(object sender, RoutedEventArgs e)
        {
            FileDialogUtil.SelectFilePath(this, AppCommon.MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH, textCertFilePath2, AppCommon.MSG_FILTER_SELECT_PIV_CERT_PEM_PATH);
        }

        private void buttonPkeyFilePath3_Click(object sender, RoutedEventArgs e)
        {
            FileDialogUtil.SelectFilePath(this, AppCommon.MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH, textPkeyFilePath3, AppCommon.MSG_FILTER_SELECT_PIV_PKEY_PEM_PATH);
        }

        private void buttonCertFilePath3_Click(object sender, RoutedEventArgs e)
        {
            FileDialogUtil.SelectFilePath(this, AppCommon.MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH, textCertFilePath3, AppCommon.MSG_FILTER_SELECT_PIV_CERT_PEM_PATH);
        }
    }
}
