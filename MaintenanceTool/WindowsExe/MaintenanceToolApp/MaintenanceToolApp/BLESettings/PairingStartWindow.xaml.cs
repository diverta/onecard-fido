using System.Windows;
using ToolAppCommon;

namespace MaintenanceToolApp.BLESettings
{
    /// <summary>
    /// PairingStartWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class PairingStartWindow : Window
    {
        // 処理実行のためのプロパティー
        private readonly BLESettingsParameter Parameter;

        // パスコードの最小／最大桁数
        private const int PASS_CODE_SIZE_MIN = 6;
        private const int PASS_CODE_SIZE_MAX = 6;

        public PairingStartWindow(BLESettingsParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
            InitializeComponent();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // パスコード入力欄をブランクにし、フォーカスを移動
            passwordBoxPasscode.Clear();
            passwordBoxPasscode.Focus();

            // この画面を、オーナー画面の中央にモード付きで表示
            Owner = ownerWindow;
            bool? b = ShowDialog();
            if (b == null) {
                return false;
            } else {
                return (bool)b;
            }
        }

        private void DoPairing52()
        {
            // 画面を閉じる
            Parameter.Passcode = string.Empty;
            TerminateWindow(true);
        }

        private void DoPairing53()
        {
            // 入力チェックがNGの場合は中止
            if (CheckEntries() == false) {
                return;
            }

            // 入力されたパスコードを設定し、画面を閉じる
            Parameter.Passcode = passwordBoxPasscode.Password;
            TerminateWindow(true);
        }

        private void TerminateWindow(bool dialogResult)
        {
            // この画面を閉じる
            DialogResult = dialogResult;
            Close();
        }

        private bool CheckEntries()
        {
            // 長さチェック
            if (PasswordBoxUtil.CheckEntrySize(passwordBoxPasscode, PASS_CODE_SIZE_MIN, PASS_CODE_SIZE_MAX, Title, AppCommon.MSG_PROMPT_INPUT_PAIRING_PASSCODE, this) == false) {
                return false;
            }

            // 数字入力チェック
            if (PasswordBoxUtil.CheckIsNumeric(passwordBoxPasscode, Title, AppCommon.MSG_PROMPT_INPUT_PAIRING_PASSCODE_NUM, this) == false) {
                return false;
            }

            return true;
        }

        //
        // イベント処理部
        // 
        private void buttonPairing52_Click(object sender, RoutedEventArgs e)
        {
            DoPairing52();
        }

        private void buttonPairing53_Click(object sender, RoutedEventArgs e)
        {
            DoPairing53();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }
    }
}
