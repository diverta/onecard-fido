using MaintenanceToolApp.CommonWindow;
using System;
using System.Windows;

namespace MaintenanceToolApp.BLESettings
{
    /// <summary>
    /// UnpairingRequestWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class UnpairingRequestWindow : Window
    {
        public UnpairingRequestWindow()
        {
            // 画面項目の初期化
            InitializeComponent();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // 画面項目の初期化
            InitFieldValue();

            // この画面を、オーナー画面の中央にモード付きで表示
            Owner = ownerWindow;
            bool? b = ShowDialog();
            if (b == null) {
                return false;
            } else {
                return (bool)b;
            }
        }

        private void InitFieldValue()
        {
            labelTitle.Content = AppCommon.MSG_BLE_UNPAIRING_PREPARATION;
            labelProgress.Content = "";
            levelIndicator.Value = 0;
            levelIndicator.Maximum = UnpairingRequestCommand.UNPAIRING_REQUEST_WAITING_SEC;
            buttonCancel.IsEnabled = false;
        }

        public void CommandDidStartWaitingForUnpair(string deviceName)
        {
            // メッセージを表示
            labelTitle.Content = string.Format(AppCommon.MSG_BLE_UNPAIRING_WAIT_DISCONNECT, deviceName);

            // Cancelボタンを使用可とする
            buttonCancel.IsEnabled = true;
        }

        public void CommandDidNotifyProgress(int remaining)
        {
            // 残り秒数をペアリング解除要求画面に表示
            labelProgress.Content = string.Format(AppCommon.MSG_BLE_UNPAIRING_WAIT_SEC_FORMAT, remaining);
            levelIndicator.Value = remaining;
        }

        public void CommandDidTerminateWaitingForUnpair()
        {
            // Cancelボタンを使用不可とする
            buttonCancel.IsEnabled = false;

            // ラベルを更新
            labelTitle.Content = AppCommon.MSG_BLE_UNPAIRING_WAIT_DISC_TIMEOUT;
        }

        public void CommandDidCancelUnpairingRequestProcess(bool success)
        {
            // コマンドクラス側での処理ステータスを戻す
            TerminateWindow(success);
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

        //
        // 閉じるボタンの無効化
        //
        protected override void OnSourceInitialized(EventArgs e)
        {
            base.OnSourceInitialized(e);
            CommonWindowUtil.DisableCloseWindowButton(this);
        }
    }
}
