using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.FIDOSettings
{
    /// <summary>
    /// FIDOSettingsWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class FIDOSettingsWindow : Window
    {
        // 処理実行のためのプロパティー
        private readonly FIDOSettingsParameter Parameter;

        public FIDOSettingsWindow(FIDOSettingsParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
            InitializeComponent();
        }

        private void TerminateWindow(bool dialogResult)
        {
            // この画面を閉じる
            DialogResult = dialogResult;
            Close();
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

        private void DoSetPinCode()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // PINコード設定画面を表示
            SetPinCodeWindow w = new SetPinCodeWindow(Parameter);
            if (w.ShowDialogWithOwner(this) == false) {
                return;
            }

            TerminateWindow(true);
        }

        private void DoReset()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // プロンプトで表示されるメッセージ
            string message = string.Format("{0}\n\n{1}", AppCommon.MSG_CLEAR_PIN_CODE, AppCommon.MSG_PROMPT_CLEAR_PIN_CODE);

            // FIDO認証情報の消去（認証器のリセット）
            // プロンプトを表示し、Yesの場合だけ処理を行う
            if (DialogUtil.DisplayPromptPopup(this, Title, message) == false) {
                return;
            }

            // 実行コマンドをパラメーターに保持
            Parameter.Command = Command.COMMAND_AUTH_RESET;
            TerminateWindow(true);
        }

        //
        // イベント処理部
        // 
        private void buttonSetPinCode_Click(object sender, RoutedEventArgs e)
        {
            DoSetPinCode();
        }

        private void buttonReset_Click(object sender, RoutedEventArgs e)
        {
            DoReset();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }
    }
}
