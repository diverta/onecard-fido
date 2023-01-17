using MaintenanceToolApp.CommonWindow;
using System;
using System.Threading.Tasks;
using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp.BLESettings
{
    /// <summary>
    /// BLESettingsWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class BLESettingsWindow : Window
    {
        // 処理実行のためのプロパティー
        private readonly BLESettingsParameter Parameter;

        public BLESettingsWindow(BLESettingsParameter param)
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

        private void DoPairing()
        {
            Task task = Task.Run(() => {
                // ペアリング対象のFIDO認証器を検索
                BLEPairingProcess.DoFindFIDOPeripheral(OnFIDOPeripheralFound);
            });

            // 進捗画面を表示
            CommonProcessingWindow.OpenForm(this);

            // FIDO認証器が見つからなかった場合は終了
            if (Parameter.BluetoothAddress == 0) {
                DialogUtil.ShowWarningMessage(this, AppCommon.PROCESS_NAME_PAIRING, Parameter.ErrorMessage);
                return;
            }

            // パスコード入力画面を表示
            PairingStartWindow w = new PairingStartWindow(Parameter);
            if (w.ShowDialogWithOwner(this) == false) {
                return;
            }

            // 実行機能を設定し、画面を閉じる
            Parameter.Command = Command.COMMAND_PAIRING;
            TerminateWindow(true);
        }

        private void OnFIDOPeripheralFound(bool found, ulong bluetoothAddress, string errorMessage)
        {
            // Bluetoothアドレス、エラーメッセージを設定
            Parameter.BluetoothAddress = bluetoothAddress;
            Parameter.ErrorMessage = errorMessage;

            if (found == false) {
                // 失敗時はログ出力
                AppLogUtil.OutputLogError(errorMessage);
            }

            Application.Current.Dispatcher.Invoke(new Action(() => {
                // 進捗画面を閉じる
                CommonProcessingWindow.NotifyTerminate();
            }));
        }

        private void DoUnpairing()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // 確認メッセージを表示し、Yesの場合だけ処理を続行する
            string message = string.Format("{0}\n\n{1}",
                AppCommon.MSG_ERASE_BONDS, AppCommon.MSG_PROMPT_ERASE_BONDS);
            if (DialogUtil.DisplayPromptPopup(this, AppCommon.MSG_TOOL_TITLE, message) == false) {
                return;
            }

            // 実行機能を設定し、画面を閉じる
            Parameter.Command = Command.COMMAND_ERASE_BONDS;
            TerminateWindow(true);
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

        private void buttonPairing_Click(object sender, RoutedEventArgs e)
        {
            DoPairing();
        }

        private void buttonUnpairing_Click(object sender, RoutedEventArgs e)
        {
            DoUnpairing();
        }
    }
}
