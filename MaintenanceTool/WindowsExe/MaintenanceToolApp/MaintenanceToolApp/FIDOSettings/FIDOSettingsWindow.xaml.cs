﻿using System.Windows;

namespace MaintenanceToolApp.FIDOSettings
{
    /// <summary>
    /// FIDOSettingsWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class FIDOSettingsWindow : Window
    {
        public FIDOSettingsWindow()
        {
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
            TerminateWindow(true);
        }


        private void DoReset()
        {
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
