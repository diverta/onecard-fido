using System.IO;
using System.Windows;
using System.Windows.Controls;
using ToolAppCommon;

namespace MaintenanceToolApp.PIV
{
    /// <summary>
    /// PIVWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class PIVWindow : Window
    {
        // 入力可能文字数
        private const int PIV_PIN_CODE_SIZE_MIN = 6;
        private const int PIV_PIN_CODE_SIZE_MAX = 8;

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

        private void DoInstallPkeyCert()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // 入力欄の内容をチェック
            if (CheckForInstallPkeyCert() == false) {
                return;
            }
        }

        private void TerminateWindow(bool dialogResult)
        {
            // この画面を閉じる
            DialogResult = dialogResult;
            Close();
        }

        //
        // PIN番号管理タブ関連の処理
        //
        private void InitTabPinManagementPinFields()
        {
            // PIN番号のテキストボックスを初期化
            passwordBoxCurPin.Password = string.Empty;
            passwordBoxNewPin.Password = string.Empty;
            passwordBoxNewPinConfirm.Password = string.Empty;

            // テキストボックスのカーソルを先頭の項目に配置
            passwordBoxCurPin.Focus();
        }

        private void ChangeLabelCaptionOfPinText(object sender)
        {
            // ラジオボタンの選択状態に応じ、入力欄のキャプションも変更する
            if (sender.Equals(radioButton1)) {
                // PIN番号を変更
                labelCurPin.Content = AppCommon.MSG_LABEL_CURRENT_PIN;
                labelNewPin.Content = AppCommon.MSG_LABEL_NEW_PIN;
                labelNewPinConfirm.Content = AppCommon.MSG_LABEL_NEW_PIN_FOR_CONFIRM;
            }
            if (sender.Equals(radioButton2)) {
                // PUK番号を変更
                labelCurPin.Content = AppCommon.MSG_LABEL_CURRENT_PUK;
                labelNewPin.Content = AppCommon.MSG_LABEL_NEW_PUK;
                labelNewPinConfirm.Content = AppCommon.MSG_LABEL_NEW_PUK_FOR_CONFIRM;
            }
            if (sender.Equals(radioButton3)) {
                // PIN番号をリセット
                labelCurPin.Content = AppCommon.MSG_LABEL_CURRENT_PUK;
                labelNewPin.Content = AppCommon.MSG_LABEL_NEW_PIN;
                labelNewPinConfirm.Content = AppCommon.MSG_LABEL_NEW_PIN_FOR_CONFIRM;
            }

            // PIN入力欄をクリアし、新しいPIN欄にフォーカスを移す
            InitTabPinManagementPinFields();
        }

        //
        // 鍵・証明書インストール時の入力チェック
        //
        private bool CheckForInstallPkeyCert()
        {
            // 入力欄のチェック
            if (CheckPathEntry(textPkeyFilePath1, AppCommon.MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH1) == false) {
                return false;
            }
            if (CheckPathEntry(textCertFilePath1, AppCommon.MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH1) == false) {
                return false;
            }
            if (CheckPathEntry(textPkeyFilePath2, AppCommon.MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH2) == false) {
                return false;
            }
            if (CheckPathEntry(textCertFilePath2, AppCommon.MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH2) == false) {
                return false;
            }
            if (CheckPathEntry(textPkeyFilePath3, AppCommon.MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH3) == false) {
                return false;
            }
            if (CheckPathEntry(textCertFilePath3, AppCommon.MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH3) == false) {
                return false;
            }
            if (CheckPinNumber(passwordBoxPin, AppCommon.MSG_LABEL_CURRENT_PIN) == false) {
                return false;
            }
            if (CheckPinNumber(passwordBoxPinConfirm, AppCommon.MSG_LABEL_CURRENT_PIN_FOR_CONFIRM) == false) {
                return false;
            }

            // 確認用PINコードのチェック
            if (CheckPinConfirm(passwordBoxPinConfirm, passwordBoxPin, AppCommon.MSG_LABEL_CURRENT_PIN_FOR_CONFIRM) == false) {
                return false;
            }

            // プロンプトを表示し、Yesの場合だけ処理を行う
            return DialogUtil.DisplayPromptPopup(this, AppCommon.MSG_INSTALL_PIV_PKEY_CERT, AppCommon.MSG_PROMPT_INSTALL_PGP_KEY);
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

        private bool CheckPinNumber(PasswordBox passwordBoxPin, string fieldName)
        {
            // 長さチェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PIV_PIN_PUK_DIGIT, fieldName);
            if (PasswordBoxUtil.CheckEntrySize(passwordBoxPin, PIV_PIN_CODE_SIZE_MIN, PIV_PIN_CODE_SIZE_MAX, Title, informativeText, this) == false) {
                return false;
            }

            // 数字チェック
            informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PIV_PIN_PUK_NUM, fieldName);
            if (PasswordBoxUtil.CheckIsNumeric(passwordBoxPin, Title, informativeText, this) == false) {
                return false;
            }

            return true;
        }

        private bool CheckPinConfirm(PasswordBox passwordBoxPinConfirm, PasswordBox passwordBoxPin, string fieldName)
        {
            // PIN番号の確認入力内容をチェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PIV_PIN_PUK_CONFIRM, fieldName);
            return PasswordBoxUtil.CompareEntry(passwordBoxPinConfirm, passwordBoxPin, Title, informativeText, this);
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

        private void buttonInstallPkeyCert_Click(object sender, RoutedEventArgs e)
        {
            DoInstallPkeyCert();
        }

        private void radioButton1_Checked(object sender, RoutedEventArgs e)
        {
            ChangeLabelCaptionOfPinText(sender);
        }

        private void radioButton2_Checked(object sender, RoutedEventArgs e)
        {
            ChangeLabelCaptionOfPinText(sender);
        }

        private void radioButton3_Checked(object sender, RoutedEventArgs e)
        {
            ChangeLabelCaptionOfPinText(sender);
        }
    }
}
