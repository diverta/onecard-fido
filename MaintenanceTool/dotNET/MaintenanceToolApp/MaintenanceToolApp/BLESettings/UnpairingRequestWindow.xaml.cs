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
