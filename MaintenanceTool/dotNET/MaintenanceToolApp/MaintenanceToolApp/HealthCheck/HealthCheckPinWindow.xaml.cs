using System.Windows;
using ToolAppCommon;

namespace MaintenanceToolApp.HealthCheck
{
    /// <summary>
    /// HealthCheckPinWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class HealthCheckPinWindow : Window
    {
        // 処理実行のためのプロパティー
        private readonly HealthCheckParameter Parameter;

        // PINコードの最小／最大桁数
        private const int PIN_CODE_SIZE_MIN = 4;
        private const int PIN_CODE_SIZE_MAX = 16;

        public HealthCheckPinWindow(HealthCheckParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
            InitializeComponent();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // PIN入力欄をブランクにし、フォーカスを移動
            passwordBoxPin.Clear();
            passwordBoxPin.Focus();

            // この画面を、オーナー画面の中央にモード付きで表示
            Owner = ownerWindow;
            bool? b = ShowDialog();
            if (b == null) {
                return false;
            } else {
                return (bool)b;
            }
        }

        private void DoOK()
        {
            // 入力チェックがNGの場合は中止
            if (CheckEntries() == false) {
                return;
            }

            // 入力されたPINを設定し、画面を閉じる
            Parameter.Pin = passwordBoxPin.Password;
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
            if (PasswordBoxUtil.CheckEntrySize(passwordBoxPin, PIN_CODE_SIZE_MIN, PIN_CODE_SIZE_MAX, Title, AppCommon.MSG_PROMPT_INPUT_HCHECK_PIN, this) == false) {
                return false;
            }

            // 数字入力チェック
            if (PasswordBoxUtil.CheckIsNumeric(passwordBoxPin, Title, AppCommon.MSG_PROMPT_INPUT_HCHECK_PIN_NUM, this) == false) {
                return false;
            }

            return true;
        }

        //
        // イベント処理部
        // 
        private void buttonOK_Click(object sender, RoutedEventArgs e)
        {
            DoOK();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }
    }
}
