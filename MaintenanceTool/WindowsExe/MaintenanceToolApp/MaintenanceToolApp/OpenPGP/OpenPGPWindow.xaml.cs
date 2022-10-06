using System.IO;
using System.Windows;
using System.Windows.Controls;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.OpenPGP
{
    /// <summary>
    /// OpenPGPWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class OpenPGPWindow : Window
    {
        // 入力可能文字数
        private const int OPENPGP_NAME_SIZE_MIN = 5;
        private const int OPENPGP_ENTRY_SIZE_MAX = 32;
        private const int OPENPGP_ADMIN_PIN_CODE_SIZE_MIN = 8;
        private const int OPENPGP_ADMIN_PIN_CODE_SIZE_MAX = 8;

        // ASCII項目入力パターン [ -z]（表示可能な半角文字はすべて許容）
        private const string OPENPGP_ENTRY_PATTERN_ASCII = "^[ -z]+$";

        // ASCII項目入力パターン [ -z]（両端の半角スペースは許容しない）
        private const string OPENPGP_ENTRY_PATTERN_NOSP_BOTH_ENDS = "^[!-z]+[ -z]*[!-z]+$";

        // メールアドレス入力パターン \w は [a-zA-Z_0-9] と等価
        private const string OPENPGP_ENTRY_PATTERN_MAIL_ADDRESS = "^\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*$";

        // 処理実行のためのプロパティー
        private readonly OpenPGPParameter Parameter;

        public OpenPGPWindow(OpenPGPParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
            InitializeComponent();

            // 画面項目の初期化
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

        private void DoInstallPGPKey()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // 入力欄の内容をチェック
            if (CheckForInstallPGPKey() == false) {
                return;
            }

            // 画面入力内容をパラメーターに設定
            Parameter.RealName = textRealName.Text;
            Parameter.MailAddress = textMailAddress.Text;
            Parameter.Comment = textComment.Text;
            Parameter.Passphrase = passwordBoxPinConfirm.Password;
            Parameter.PubkeyFolderPath = textPubkeyFolderPath.Text;
            Parameter.BackupFolderPath = textBackupFolderPath.Text;

            // TODO: 仮の実装です。
            AppLogUtil.OutputLogDebug(Parameter.ToString());
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
            GetSelectedPinCommandValue(Parameter);
            Parameter.CurrentPin = passwordBoxCurPin .Password;
            Parameter.NewPin = passwordBoxNewPinConfirm.Password;

            // TODO: 仮の実装です。
            AppLogUtil.OutputLogDebug(Parameter.ToString());
        }

        private void TerminateWindow(bool dialogResult)
        {
            // この画面を閉じる
            DialogResult = dialogResult;
            Close();
        }

        //
        // 画面初期化処理
        //
        private void InitFieldValue()
        {
            // PGP鍵管理タブ内の入力項目を初期化
            InitTabPGPKeyManagement();

            // PIN番号管理タブ内の入力項目を初期化
            InitTabPinManagement();
        }

        //
        // PGP鍵管理タブ関連の処理
        //
        private void InitTabPGPKeyManagement()
        {
            // テキストボックスの初期化
            InitTabPGPKeyPathFields();
            InitTabPGPKeyEntryFields();
        }

        private void InitTabPGPKeyPathFields()
        {
            // ファイルパスのテキストボックスを初期化
            textPubkeyFolderPath.Text = string.Empty;
            textBackupFolderPath.Text = string.Empty;
        }

        private void InitTabPGPKeyEntryFields()
        {
            // テキストボックスを初期化
            textRealName.Text = string.Empty;
            textMailAddress.Text = string.Empty;
            textComment.Text = string.Empty;
            passwordBoxPin.Password = string.Empty;
            passwordBoxPinConfirm.Password = string.Empty;

            // テキストボックスのカーソルを先頭の項目に配置
            textRealName.Focus();
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
                labelCurPin.Content = AppCommon.MSG_LABEL_ITEM_CUR_PIN;
                labelNewPin.Content = AppCommon.MSG_LABEL_ITEM_NEW_PIN;
            }
            if (sender.Equals(radioButton2)) {
                // 管理用PIN番号を変更
                labelCurPin.Content = AppCommon.MSG_LABEL_ITEM_CUR_ADMPIN;
                labelNewPin.Content = AppCommon.MSG_LABEL_ITEM_NEW_ADMPIN;
            }
            if (sender.Equals(radioButton3)) {
                // PIN番号をリセット
                labelCurPin.Content = AppCommon.MSG_LABEL_ITEM_CUR_ADMPIN;
                labelNewPin.Content = AppCommon.MSG_LABEL_ITEM_NEW_PIN;
            }
            if (sender.Equals(radioButton4)) {
                // リセットコードを変更
                labelCurPin.Content = AppCommon.MSG_LABEL_ITEM_CUR_ADMPIN;
                labelNewPin.Content = AppCommon.MSG_LABEL_ITEM_NEW_RESET_CODE;
            }
            if (sender.Equals(radioButton5)) {
                // リセットコードでPIN番号をリセット
                labelCurPin.Content = AppCommon.MSG_LABEL_ITEM_CUR_RESET_CODE;
                labelNewPin.Content = AppCommon.MSG_LABEL_ITEM_NEW_PIN;
            }

            // 確認欄のキャプションを設定
            labelNewPinConfirm.Content = string.Format(AppCommon.MSG_FORMAT_OPENPGP_ITEM_FOR_CONF, labelNewPin.Content);

            // PIN入力欄をクリアし、新しいPIN欄にフォーカスを移す
            InitTabPinManagementPinFields();
        }

        //
        // PGP秘密鍵インストール時の入力チェック
        //
        private bool CheckForInstallPGPKey()
        {
            // 入力欄のチェック
            if (CheckMustEntry(textRealName, AppCommon.MSG_LABEL_PGP_REAL_NAME, OPENPGP_NAME_SIZE_MIN, OPENPGP_ENTRY_SIZE_MAX) == false) {
                return false;
            }
            if (CheckAsciiEntry(textRealName, AppCommon.MSG_LABEL_PGP_REAL_NAME) == false) {
                return false;
            }
            if (CheckEntryNoSpaceExistOnBothEnds(textRealName, AppCommon.MSG_LABEL_PGP_REAL_NAME) == false) {
                return false;
            }
            if (CheckMustEntry(textMailAddress, AppCommon.MSG_LABEL_PGP_MAIL_ADDRESS, 1, OPENPGP_ENTRY_SIZE_MAX) == false) {
                return false;
            }
            if (CheckAddressEntry(textMailAddress, AppCommon.MSG_LABEL_PGP_MAIL_ADDRESS) == false) {
                return false;
            }
            if (CheckMustEntry(textComment, AppCommon.MSG_LABEL_PGP_COMMENT, 1, OPENPGP_ENTRY_SIZE_MAX) == false) {
                return false;
            }
            if (CheckAsciiEntry(textComment, AppCommon.MSG_LABEL_PGP_COMMENT) == false) {
                return false;
            }
            if (CheckEntryNoSpaceExistOnBothEnds(textComment, AppCommon.MSG_LABEL_PGP_COMMENT) == false) {
                return false;
            }
            if (CheckPathEntry(textPubkeyFolderPath, AppCommon.MSG_PROMPT_SELECT_PGP_PUBKEY_FOLDER) == false) {
                return false;
            }
            if (CheckPathEntry(textBackupFolderPath, AppCommon.MSG_PROMPT_SELECT_PGP_BACKUP_FOLDER) == false) {
                return false;
            }
            if (CheckPinNumber(passwordBoxPin, AppCommon.MSG_LABEL_PGP_ADMIN_PIN) == false) {
                return false;
            }
            if (CheckPinNumber(passwordBoxPinConfirm, AppCommon.MSG_LABEL_PGP_ADMIN_PIN_CONFIRM) == false) {
                return false;
            }

            // 確認用PINコードのチェック
            if (CheckPinConfirm(passwordBoxPinConfirm, passwordBoxPin, AppCommon.MSG_LABEL_PGP_ADMIN_PIN_CONFIRM) == false) {
                return false;
            }

            // プロンプトを表示し、Yesの場合だけ処理を行う
            return DialogUtil.DisplayPromptPopup(this, AppCommon.MSG_OPENPGP_INSTALL_PGP_KEY, AppCommon.MSG_PROMPT_INSTALL_PGP_KEY);
        }

        private bool CheckMustEntry(TextBox text, string fieldName, int sizeMin, int sizeMax)
        {
            // 必須チェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_MUST_ENTRY, fieldName);
            if (text.Text.Length == 0) {
                DialogUtil.ShowWarningMessage(this, Title, informativeText);
                text.Focus();
                return false;
            }

            // 長さチェック
            informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ENTRY_DIGIT, fieldName, sizeMin, sizeMax);
            if (TextBoxUtil.CheckEntrySize(text, sizeMin, sizeMax, Title, informativeText, this) == false) {
                return false;
            }

            return true;
        }

        private bool CheckAsciiEntry(TextBox text, string fieldName)
        {
            // 入力パターンチェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ASCII_ENTRY, fieldName);
            if (TextBoxUtil.CheckValueWithPattern(text, OPENPGP_ENTRY_PATTERN_ASCII, Title, informativeText, this) == false) {
                return false;
            }
            return true;
        }

        private bool CheckAddressEntry(TextBox text, string fieldName)
        {
            // 入力パターンチェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ADDRESS_ENTRY, fieldName);
            if (TextBoxUtil.CheckValueWithPattern(text, OPENPGP_ENTRY_PATTERN_MAIL_ADDRESS, Title, informativeText, this) == false) {
                return false;
            }
            return true;
        }

        private bool CheckEntryNoSpaceExistOnBothEnds(TextBox text, string fieldName)
        {
            // 先頭または末尾に半角スペース文字が入っている場合はエラー
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ENTRY_NOSP_BOTH_ENDS, fieldName);
            if (TextBoxUtil.CheckValueWithPattern(text, OPENPGP_ENTRY_PATTERN_NOSP_BOTH_ENDS, Title, informativeText, this) == false) {
                return false;
            }
            return true;
        }

        private bool CheckPathEntry(TextBox text, string messageIfError)
        {
            // 必須チェック（ただし、入力できないのでフォーカスは移動しない）
            if (text.Text.Length == 0) {
                DialogUtil.ShowWarningMessage(this, Title, messageIfError);
                return false;
            }

            // 入力されたファイルパスが存在しない場合は終了
            string path = text.Text;
            if (Directory.Exists(path) == false) {
                DialogUtil.ShowWarningMessage(this, Title, messageIfError);
                return false;
            }

            return true;
        }

        private bool CheckPinNumber(PasswordBox passwordBoxPin, string fieldName)
        {
            // 長さチェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ADMIN_PIN_DIGIT, fieldName);
            if (PasswordBoxUtil.CheckEntrySize(passwordBoxPin, OPENPGP_ADMIN_PIN_CODE_SIZE_MIN, OPENPGP_ADMIN_PIN_CODE_SIZE_MAX, Title, informativeText, this) == false) {
                return false;
            }

            // 数字チェック
            informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ADMIN_PIN_NUM, fieldName);
            if (PasswordBoxUtil.CheckIsNumeric(passwordBoxPin, Title, informativeText, this) == false) {
                return false;
            }

            return true;
        }

        private bool CheckPinConfirm(PasswordBox passwordBoxPinConfirm, PasswordBox passwordBoxPin, string fieldName)
        {
            // PIN番号の確認入力内容をチェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ADMIN_PIN_CONFIRM, fieldName);
            return PasswordBoxUtil.CompareEntry(passwordBoxPinConfirm, passwordBoxPin, Title, informativeText, this);
        }

        //
        // PIN番号変更時の入力チェック
        //
        private bool CheckForPerformPinCommand()
        {
            // PIN番号管理タブのラジオボタン選択状態に
            // 応じたコマンド／機能名称を取得
            OpenPGPParameter param = new OpenPGPParameter();
            GetSelectedPinCommandValue(param);

            // チェック用パラメーターの設定
            string msgCurPin = "";
            string msgNewPin = "";
            int minSizeCurPin = 0;
            int minSizeNewPin = 0;
            switch (param.SelectedPinCommand) {
            case Command.COMMAND_OPENPGP_CHANGE_PIN:
                msgCurPin = AppCommon.MSG_LABEL_ITEM_CUR_PIN;
                minSizeCurPin = 6;
                msgNewPin = AppCommon.MSG_LABEL_ITEM_NEW_PIN;
                minSizeNewPin = 6;
                break;
            case Command.COMMAND_OPENPGP_CHANGE_ADMIN_PIN:
                msgCurPin = AppCommon.MSG_LABEL_ITEM_CUR_ADMPIN;
                minSizeCurPin = 8;
                msgNewPin = AppCommon.MSG_LABEL_ITEM_NEW_ADMPIN;
                minSizeNewPin = 8;
                break;
            case Command.COMMAND_OPENPGP_UNBLOCK_PIN:
                msgCurPin = AppCommon.MSG_LABEL_ITEM_CUR_ADMPIN;
                minSizeCurPin = 8;
                msgNewPin = AppCommon.MSG_LABEL_ITEM_NEW_PIN;
                minSizeNewPin = 6;
                break;
            case Command.COMMAND_OPENPGP_SET_RESET_CODE:
                msgCurPin = AppCommon.MSG_LABEL_ITEM_CUR_ADMPIN;
                minSizeCurPin = 8;
                msgNewPin = AppCommon.MSG_LABEL_ITEM_NEW_RESET_CODE;
                minSizeNewPin = 8;
                break;
            case Command.COMMAND_OPENPGP_UNBLOCK:
                msgCurPin = AppCommon.MSG_LABEL_ITEM_CUR_RESET_CODE;
                minSizeCurPin = 8;
                msgNewPin = AppCommon.MSG_LABEL_ITEM_NEW_PIN;
                minSizeNewPin = 6;
                break;
            default:
                break;
            }

            // 現在のPINをチェック
            if (CheckPinNumberForPinCommand(passwordBoxCurPin, msgCurPin, minSizeCurPin) == false) {
                return false;
            }

            // 新しいPINをチェック
            if (CheckPinNumberForPinCommand(passwordBoxNewPin, msgNewPin, minSizeNewPin) == false) {
                return false;
            }

            // 確認用PINのラベル
            string msgNewPinConf = string.Format(AppCommon.MSG_FORMAT_OPENPGP_ITEM_FOR_CONFIRM, msgNewPin);

            // 確認用PINをチェック
            // 確認用PINコードのチェック
            if (CheckPinConfirm(passwordBoxNewPinConfirm, passwordBoxNewPin, msgNewPinConf) == false) {
                return false;
            }

            // プロンプトを表示し、Yesの場合だけ処理を行う
            string caption = string.Format(AppCommon.MSG_FORMAT_OPENPGP_WILL_PROCESS, param.SelectedPinCommandName);
            return DialogUtil.DisplayPromptPopup(this, caption, AppCommon.MSG_PROMPT_OPENPGP_PIN_COMMAND);
        }

        private bool CheckPinNumberForPinCommand(PasswordBox passwordBoxPin, string fieldName, int size_min)
        {
            // 長さチェック
            string informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_PIN_DIGIT, fieldName, size_min);
            if (PasswordBoxUtil.CheckEntrySize(passwordBoxPin, size_min, size_min, Title, informativeText, this) == false) {
                return false;
            }

            // 数字チェック
            informativeText = string.Format(AppCommon.MSG_PROMPT_INPUT_PGP_ADMIN_PIN_NUM, fieldName);
            if (PasswordBoxUtil.CheckIsNumeric(passwordBoxPin, Title, informativeText, this) == false) {
                return false;
            }

            return true;
        }

        //
        // PIN番号管理タブのラジオボタン選択状態に
        // 応じたコマンド／機能名称を設定
        //
        private void GetSelectedPinCommandValue(OpenPGPParameter param)
        {
            if (RadioButtonIsChecked(radioButton1)) {
                // PIN番号を変更
                param.SelectedPinCommand = Command.COMMAND_OPENPGP_CHANGE_PIN;
                param.SelectedPinCommandName = AppCommon.MSG_LABEL_COMMAND_OPENPGP_CHANGE_PIN;
            }
            if (RadioButtonIsChecked(radioButton2)) {
                // 管理用PIN番号を変更
                param.SelectedPinCommand = Command.COMMAND_OPENPGP_CHANGE_ADMIN_PIN;
                param.SelectedPinCommandName = AppCommon.MSG_LABEL_COMMAND_OPENPGP_CHANGE_ADMIN_PIN;
            }
            if (RadioButtonIsChecked(radioButton3)) {
                // PIN番号をリセット
                param.SelectedPinCommand = Command.COMMAND_OPENPGP_UNBLOCK_PIN;
                param.SelectedPinCommandName = AppCommon.MSG_LABEL_COMMAND_OPENPGP_UNBLOCK_PIN;
            }
            if (RadioButtonIsChecked(radioButton4)) {
                // リセットコードを変更
                param.SelectedPinCommand = Command.COMMAND_OPENPGP_SET_RESET_CODE;
                param.SelectedPinCommandName = AppCommon.MSG_LABEL_COMMAND_OPENPGP_SET_RESET_CODE;
            }
            if (RadioButtonIsChecked(radioButton5)) {
                // リセットコードでPIN番号をリセット
                param.SelectedPinCommand = Command.COMMAND_OPENPGP_UNBLOCK;
                param.SelectedPinCommandName = AppCommon.MSG_LABEL_COMMAND_OPENPGP_UNBLOCK;
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

        private void buttonPubkeyFolderPath_Click(object sender, RoutedEventArgs e)
        {
            FileDialogUtil.SelectFolderPath(this, AppCommon.MSG_PROMPT_SELECT_PGP_PUBKEY_FOLDER, textPubkeyFolderPath);
        }

        private void buttonBackupFolderPath_Click(object sender, RoutedEventArgs e)
        {
            FileDialogUtil.SelectFolderPath(this, AppCommon.MSG_PROMPT_SELECT_PGP_BACKUP_FOLDER, textBackupFolderPath);
        }

        private void buttonInstallPGPKey_Click(object sender, RoutedEventArgs e)
        {
            DoInstallPGPKey();
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

        private void radioButton4_Checked(object sender, RoutedEventArgs e)
        {
            ChangeLabelCaptionOfPinText(sender);
        }

        private void radioButton5_Checked(object sender, RoutedEventArgs e)
        {
            ChangeLabelCaptionOfPinText(sender);
        }

        private void buttonPerformPinCommand_Click(object sender, RoutedEventArgs e)
        {
            DoPerformPinCommand();
        }
    }
}
