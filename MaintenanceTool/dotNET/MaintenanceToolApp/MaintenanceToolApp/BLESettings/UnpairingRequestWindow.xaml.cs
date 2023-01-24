using MaintenanceToolApp.CommonWindow;
using System;
using System.Threading.Tasks;
using System.Windows;

namespace MaintenanceToolApp.BLESettings
{
    /// <summary>
    /// UnpairingRequestWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class UnpairingRequestWindow : Window
    {
        // 時刻設定処理の参照を保持
        private BLESettingsParameter Parameter = null!;

        public UnpairingRequestWindow(BLESettingsParameter parameter)
        {
            // パラメーターの参照を保持
            Parameter = parameter;

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
