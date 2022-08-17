﻿using System.Windows;

namespace MaintenanceToolApp
{
    /// <summary>
    /// UtilityWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class UtilityWindow : Window
    {
        public UtilityWindow()
        {
            InitializeComponent();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // ユーティリティー画面を、オーナー画面の中央にモード付きで表示
            Owner = ownerWindow;
            bool? b = ShowDialog();
            if (b == null) {
                return false;
            } else {
                return (bool)b;
            }
        }

        private void DoToolVersionInfo()
        {
            // バージョン参照画面を開く
            ToolVersionWindow w = new ToolVersionWindow();
            w.ShowDialogWithOwner(this);

            // 画面を閉じる
            TerminateWindow(false);
        }

        private void DoViewLogFile()
        {
            // 実行するユーティリティー機能の名称を設定し、画面を閉じる
            UtilityProcess.SetCommandTitle(AppCommon.PROCESS_NAME_VIEW_LOG_FILE);
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

        private void buttonToolVersionInfo_Click(object sender, RoutedEventArgs e)
        {
            DoToolVersionInfo();
        }

        private void buttonViewLogFile_Click(object sender, RoutedEventArgs e)
        {
            DoViewLogFile();
        }

        private void buttonCancel_Click(object sender, RoutedEventArgs e)
        {
            TerminateWindow(false);
        }
    }
}
