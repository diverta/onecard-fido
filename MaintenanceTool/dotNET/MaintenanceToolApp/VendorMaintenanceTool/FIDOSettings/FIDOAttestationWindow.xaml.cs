using MaintenanceToolApp;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using ToolAppCommon;
using VendorMaintenanceTool.VendorFunction;
using static VendorMaintenanceTool.VendorAppCommon;

namespace VendorMaintenanceTool.FIDOSettings
{
    /// <summary>
    /// FIDOAttestationWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class FIDOAttestationWindow : Window
    {
        // 処理実行のためのプロパティー
        private readonly VendorFunctionParameter Parameter;

        internal FIDOAttestationWindow(VendorFunctionParameter param)
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

        private void TerminateWindow(bool dialogResult)
        {
            // この画面を閉じる
            DialogResult = dialogResult;
            Close();
        }

        private void DoInstall()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // 入力欄の内容をチェック
            if (CheckForInstallKeyCert() == false) {
                return;
            }

            // 入力されたパスを設定し、画面を閉じる
            Parameter.KeyPath = textKeyPath.Text;
            Parameter.CertPath = textCertPath.Text;
            TerminateWindow(true);
        }

        private bool CheckForInstallKeyCert()
        {
            // 入力欄のチェック
            if (CheckPathEntry(textKeyPath, MSG_PROMPT_SELECT_PKEY_PATH) == false) {
                return false;
            }
            if (CheckPathEntry(textCertPath, MSG_PROMPT_SELECT_CRT_PATH) == false) {
                return false;
            }

            // プロンプトを表示し、Yesの場合だけ処理を行う
            return DialogUtil.DisplayPromptPopup(this, MSG_INSTALL_SKEY_CERT, MSG_PROMPT_INSTALL_SKEY_CERT);
        }

        private bool CheckPathEntry(TextBox text, string messageIfError)
        {
            // 必須チェック（ただし、入力できないのでフォーカスは移動しない）
            if (text.Text.Length == 0) {
                DialogUtil.ShowWarningMessage(this, Title, messageIfError);
                return false;
            }

            // 入力されたファイルが存在しない場合は終了
            string path = text.Text;
            if (File.Exists(path) == false) {
                DialogUtil.ShowWarningMessage(this, Title, messageIfError);
                return false;
            }

            return true;
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
            DoInstall();
        }

        private void buttonSelectKeyPath_Click(object sender, RoutedEventArgs e)
        {
            FileDialogUtil.SelectFilePath(this, MSG_PROMPT_SELECT_PKEY_PATH, textKeyPath, MSG_FILTER_SELECT_FIDO_PKEY_PEM_PATH);
        }

        private void buttonSelectCertPath_Click(object sender, RoutedEventArgs e)
        {
            FileDialogUtil.SelectFilePath(this, MSG_PROMPT_SELECT_CRT_PATH, textCertPath, MSG_FILTER_SELECT_FIDO_CERT_CRT_PATH);
        }
    }
}
