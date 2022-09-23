﻿using System.Windows;

namespace MaintenanceToolApp.FIDOSettings
{
    /// <summary>
    /// SetPinCodeWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class SetPinCodeWindow : Window
    {
        public SetPinCodeWindow()
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

        private void DoSetPin()
        {
            TerminateWindow(true);
        }

        private void DoChangePin()
        {
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
        private void buttonSetPin_Click(object sender, RoutedEventArgs e)
        {
            DoSetPin();
        }

        private void buttonChangePin_Click(object sender, RoutedEventArgs e)
        {
            DoChangePin();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }
    }
}
