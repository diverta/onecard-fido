﻿using System.Windows;
using static MaintenanceToolApp.AppDefine.Command;

namespace MaintenanceTool.OATH
{
    /// <summary>
    /// AccountSelectWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class AccountSelectWindow : Window
    {
        // 処理パラメーターの参照を保持
        private readonly OATHParameter Parameter = null!;

        public AccountSelectWindow(OATHParameter parameter)
        {
            // 処理パラメーターの参照を保持
            Parameter = parameter;

            // 画面項目の初期化
            InitializeComponent();
            InitFieldValue();
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

        //
        // 画面初期化処理
        //
        private void InitFieldValue()
        {
            // 実行機能に応じ、表示文言を変更
            if (Parameter.Command == COMMAND_OATH_SHOW_PASSWORD) {
                Title = "ワンタイムパスワードを参照するアカウントの選択";
                labelCaption.Content = "ワンタイムパスワードを参照したいアカウントを、\n下のリストから選択して下さい。";
            }
            if (Parameter.Command == COMMAND_OATH_DELETE_ACCOUNT) {
                Title = "削除するアカウントの選択";
                labelCaption.Content = "認証器から削除したいアカウントを、\n下のリストから選択して下さい。";
            }

            // 選択ボタンを使用不可
            buttonSelect.IsEnabled = false;
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
    }
}
