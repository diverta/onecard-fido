using MaintenanceToolApp;
using System.Windows;

namespace VendorMaintenanceTool.FIDOSettings
{
    /// <summary>
    /// FIDOAttestationWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class FIDOAttestationWindow : Window
    {
        public FIDOAttestationWindow()
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
        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }

        private void ButtonInstall_Click(object sender, RoutedEventArgs e)
        {
            // TODO: 仮の実装です。
            DialogUtil.ShowWarningMessage(this, Title, AppCommon.MSG_CMDTST_MENU_NOT_SUPPORTED);
        }

        private void buttonSelectKeyPath_Click(object sender, RoutedEventArgs e)
        {
            FileDialogUtil.SelectFilePath(this, VendorAppCommon.MSG_PROMPT_SELECT_PKEY_PATH, textKeyPath, VendorAppCommon.MSG_FILTER_SELECT_FIDO_PKEY_PEM_PATH);
        }

        private void buttonSelectCertPath_Click(object sender, RoutedEventArgs e)
        {
            FileDialogUtil.SelectFilePath(this, VendorAppCommon.MSG_PROMPT_SELECT_CRT_PATH, textCertPath, VendorAppCommon.MSG_FILTER_SELECT_FIDO_CERT_CRT_PATH);
        }
    }
}
