using MaintenanceToolApp.CommonWindow;
using System;
using System.IO;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.PIV
{
    /// <summary>
    /// PIVWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class PIVWindow : Window
    {
        // PIV処理の参照を保持
        private readonly PIVProcess Process = null!;

        // 入力可能文字数
        private const int PIV_PIN_CODE_SIZE_MIN = 6;
        private const int PIV_PIN_CODE_SIZE_MAX = 8;

        public PIVWindow()
        {
            // PIV処理クラスの参照を保持
            Process = new PIVProcess();

            // 画面項目の初期化
            InitializeComponent();
            InitFieldValue();
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

            // 画面入力内容をパラメーターに設定
            PIVParameter param = new PIVParameter();
            param.PkeyFilePath1 = textPkeyFilePath1.Text;
            param.CertFilePath1 = textCertFilePath1.Text;
            param.PkeyFilePath2 = textPkeyFilePath2.Text;
            param.CertFilePath2 = textCertFilePath2.Text;
            param.PkeyFilePath3 = textPkeyFilePath3.Text;
            param.CertFilePath3 = textCertFilePath3.Text;
            param.AuthPin = passwordBoxPinConfirm.Password;

            // コマンドを実行
            param.Command = Command.COMMAND_CCID_PIV_IMPORT_KEY;
            param.CommandTitle = AppCommon.MSG_PIV_INSTALL_PKEY_CERT;
            DoPIVProcess(param);
        }

        private void DoPerformPinCommand()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // 入力欄の内容をチェック
            if (CheckForPerformPinCommand() == false) {
                return;
            }

            // 画面入力内容をパラメーターに設定
            PIVParameter param = new PIVParameter();
            GetSelectedPinCommandValue(param);
            param.CurrentPin = passwordBoxCurPin.Password;
            param.NewPin = passwordBoxNewPinConfirm.Password;

            // コマンドを実行
            DoPIVProcess(param);
        }

        private void DoPIVStatus()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // コマンドを実行
            PIVParameter param = new PIVParameter();
            param.Command = Command.COMMAND_CCID_PIV_STATUS;
            param.CommandTitle = AppCommon.MSG_PIV_STATUS;
            DoPIVProcess(param);
        }

        private void DoPIVReset()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // プロンプトで表示されるタイトル
            string title = string.Format(
                AppCommon.MSG_FORMAT_WILL_PROCESS,
                AppCommon.MSG_PIV_CLEAR_SETTING);

            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (DialogUtil.DisplayPromptPopup(this, title, AppCommon.MSG_PROMPT_PIV_CLEAR_SETTING) == false) {
                return;
            }

            // コマンドを実行
            PIVParameter param = new PIVParameter();
            param.Command = Command.COMMAND_CCID_PIV_RESET;
            param.CommandTitle = AppCommon.MSG_PIV_CLEAR_SETTING;
            DoPIVProcess(param);
        }

        private void DoFirmwareReset()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // コマンドを実行
            PIVParameter param = new PIVParameter();
            param.Command = Command.COMMAND_HID_FIRMWARE_RESET;
            param.CommandTitle = AppCommon.PROCESS_NAME_FIRMWARE_RESET;
            DoPIVProcess(param);
        }

        private void TerminateWindow(bool dialogResult)
        {
            // この画面を閉じる
            DialogResult = dialogResult;
            Close();
        }

        //
        // コマンド実行指示～完了後の処理
        //
        private void DoPIVProcess(PIVParameter param)
        {
            Task task = Task.Run(() => {
                // コマンドを実行
                Process.DoPIVProcess(param, new PIVProcess.HandlerOnNotifyProcessTerminated(OnPIVProcessTerminated));
            });

            // 進捗画面を表示
            CommonProcessingWindow.OpenForm(this);

            // メッセージをポップアップ表示
            if (param.CommandSuccess) {
                DialogUtil.ShowInfoMessage(this, Title, param.ResultMessage);
            } else {
                DialogUtil.ShowWarningMessage(this, Title, param.ResultMessage);
            }

            // 全ての入力欄をクリア
            ClearEntry(param.Command, param.CommandSuccess);
        }

        private void ClearEntry(Command command, bool commandSuccess)
        {
            switch (command) {
            case Command.COMMAND_CCID_PIV_IMPORT_KEY:
                if (commandSuccess) {
                    InitTabPkeyCertPathFields();
                    InitTabPkeyCertEntryFields();
                }
                break;
            case Command.COMMAND_CCID_PIV_CHANGE_PIN:
            case Command.COMMAND_CCID_PIV_CHANGE_PUK:
            case Command.COMMAND_CCID_PIV_UNBLOCK_PIN:
                if (commandSuccess) {
                    InitTabPinManagementPinFields();
                }
                break;
            default:
                break;
            }
        }

        private void OnPIVProcessTerminated(PIVParameter param)
        {
            Application.Current.Dispatcher.Invoke(new Action(() => {
                // 進捗画面を閉じる
                CommonProcessingWindow.NotifyTerminate();
            }));
        }

        //
        // 画面初期化処理
        //
        private void InitFieldValue()
        {
            // 鍵・証明書管理タブ内の入力項目を初期化
            InitTabPkeyCertManagement();

            // PIN番号管理タブ内の入力項目を初期化
            InitTabPinManagement();
        }

        //
        // 鍵・証明書管理タブ関連の処理
        //
        private void InitTabPkeyCertManagement()
        {
            // テキストボックスの初期化
            InitTabPkeyCertPathFields();
            InitTabPkeyCertEntryFields();
        }

        private void InitTabPkeyCertPathFields()
        {
            // ファイルパスのテキストボックスを初期化
            textPkeyFilePath1.Text = string.Empty;
            textCertFilePath1.Text = string.Empty;
            textPkeyFilePath2.Text = string.Empty;
            textCertFilePath2.Text = string.Empty;
            textPkeyFilePath3.Text = string.Empty;
            textCertFilePath3.Text = string.Empty;
        }

        private void InitTabPkeyCertEntryFields()
        {
            // テキストボックスを初期化
            passwordBoxPin.Password = string.Empty;
            passwordBoxPinConfirm.Password = string.Empty;

            // テキストボックスのカーソルを先頭の項目に配置
            passwordBoxPin.Focus();
        }

        //
        // PIN番号管理タブ関連の処理
        //
        private void InitTabPinManagement()
        {
            // ラジオボタンの初期化
            InitButtonPinCommandsWithDefault(radioButton1);
        }

        private void InitButtonPinCommandsWithDefault(RadioButton radioButton)
        {
            // 「実行する機能」のラジオボタン「PIN番号を変更」を選択状態にする
            radioButton1.IsChecked = true;
            ChangeLabelCaptionOfPinText(radioButton);
        }

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
        // PIN番号変更時の入力チェック
        //
        private bool CheckForPerformPinCommand()
        {
            // PIN番号管理タブのラジオボタン選択状態に
            // 応じたコマンド／機能名称を取得
            PIVParameter param = new PIVParameter();
            GetSelectedPinCommandValue(param);

            // チェック用パラメーターの設定
            string msgCurPin = "";
            string msgNewPin = "";
            string msgNewPinConf = "";
            switch (param.Command) {
            case Command.COMMAND_CCID_PIV_CHANGE_PIN:
                msgCurPin = AppCommon.MSG_LABEL_CURRENT_PIN;
                msgNewPin = AppCommon.MSG_LABEL_NEW_PIN;
                msgNewPinConf = AppCommon.MSG_LABEL_NEW_PIN_FOR_CONFIRM;
                break;
            case Command.COMMAND_CCID_PIV_CHANGE_PUK:
                msgCurPin = AppCommon.MSG_LABEL_CURRENT_PUK;
                msgNewPin = AppCommon.MSG_LABEL_NEW_PUK;
                msgNewPinConf = AppCommon.MSG_LABEL_NEW_PUK_FOR_CONFIRM;
                break;
            case Command.COMMAND_CCID_PIV_UNBLOCK_PIN:
                msgCurPin = AppCommon.MSG_LABEL_CURRENT_PUK;
                msgNewPin = AppCommon.MSG_LABEL_NEW_PIN;
                msgNewPinConf = AppCommon.MSG_LABEL_NEW_PIN_FOR_CONFIRM;
                break;
            default:
                break;
            }

            // 現在のPINをチェック
            if (CheckPinNumber(passwordBoxCurPin, msgCurPin) == false) {
                return false;
            }

            // 新しいPINをチェック
            if (CheckPinNumber(passwordBoxNewPin, msgNewPin) == false) {
                return false;
            }

            // 確認用PINをチェック
            if (CheckPinNumber(passwordBoxNewPinConfirm, msgNewPinConf) == false) {
                return false;
            }
            if (CheckPinConfirm(passwordBoxNewPinConfirm, passwordBoxNewPin, msgNewPinConf) == false) {
                return false;
            }

            // プロンプトを表示し、Yesの場合だけ処理を行う
            string caption = string.Format(AppCommon.MSG_FORMAT_WILL_PROCESS, param.CommandTitle);
            string message = string.Format(AppCommon.MSG_FORMAT_PROCESS_INFORMATIVE, param.CommandDesc);
            return DialogUtil.DisplayPromptPopup(this, caption, message);
        }

        //
        // PIN番号管理タブのラジオボタン選択状態に
        // 応じたコマンド／機能名称を設定
        //
        private void GetSelectedPinCommandValue(PIVParameter param)
        {
            if (RadioButtonIsChecked(radioButton1)) {
                // PIN番号を変更
                param.Command = Command.COMMAND_CCID_PIV_CHANGE_PIN;
                param.CommandTitle = AppCommon.MSG_PIV_CHANGE_PIN_NUMBER;
                param.CommandDesc = AppCommon.MSG_DESC_PIV_CHANGE_PIN_NUMBER;
            }
            if (RadioButtonIsChecked(radioButton2)) {
                // PUK番号を変更
                param.Command = Command.COMMAND_CCID_PIV_CHANGE_PUK;
                param.CommandTitle = AppCommon.MSG_PIV_CHANGE_PUK_NUMBER;
                param.CommandDesc = AppCommon.MSG_DESC_PIV_CHANGE_PUK_NUMBER;
            }
            if (RadioButtonIsChecked(radioButton3)) {
                // PIN番号をリセット
                param.Command = Command.COMMAND_CCID_PIV_UNBLOCK_PIN;
                param.CommandTitle = AppCommon.MSG_PIV_RESET_PIN_NUMBER;
                param.CommandDesc = AppCommon.MSG_DESC_PIV_RESET_PIN_NUMBER;
            }
        }

        private static bool RadioButtonIsChecked(RadioButton radioButton)
        {
            if (radioButton.IsChecked == true) {
                return true;
            } else {
                return false;
            }
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

        private void buttonPerformPinCommand_Click(object sender, RoutedEventArgs e)
        {
            DoPerformPinCommand();
        }

        private void buttonPIVStatus_Click(object sender, RoutedEventArgs e)
        {
            DoPIVStatus();
        }

        private void buttonPIVReset_Click(object sender, RoutedEventArgs e)
        {
            DoPIVReset();
        }

        private void buttonFirmwareReset_Click(object sender, RoutedEventArgs e)
        {
            DoFirmwareReset();
        }
    }
}
