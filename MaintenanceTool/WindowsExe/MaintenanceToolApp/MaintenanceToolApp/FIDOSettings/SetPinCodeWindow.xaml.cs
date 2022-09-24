﻿using System.Windows;
using ToolAppCommon;

namespace MaintenanceToolApp.FIDOSettings
{
    /// <summary>
    /// SetPinCodeWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class SetPinCodeWindow : Window
    {
        // PINコードの最小／最大桁数
        private const int PIN_CODE_SIZE_MIN = 4;
        private const int PIN_CODE_SIZE_MAX = 16;

        public SetPinCodeWindow()
        {
            InitializeComponent();
        }

        public bool ShowDialogWithOwner(Window ownerWindow)
        {
            // PIN入力欄をブランクにし、フォーカスを移動
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

        private void DoSetPin()
        {
            // 入力チェックがNGの場合は中止
            if (CheckEntries() == false) {
                return;
            }

            TerminateWindow(true);
        }

        private void DoChangePin()
        {
            // 入力チェックがNGの場合は中止
            if (CheckEntries() == false) {
                return;
            }

            TerminateWindow(true);
        }

        private void TerminateWindow(bool dialogResult)
        {
            // この画面を閉じる
            DialogResult = dialogResult;
            Close();
        }

        //
        // 画面項目関連
        //
        private void InitFieldValue()
        {
            // PIN入力欄をブランクにし、フォーカスを移動
            passwordBoxPin.Clear();
            passwordBoxPinConfirm.Clear();
            passwordBoxPinOld.Clear();
            passwordBoxPin.Focus();
        }

        private bool CheckEntries()
        {
            // 長さチェック
            if (PasswordBoxUtil.CheckEntrySize(passwordBoxPin, PIN_CODE_SIZE_MIN, PIN_CODE_SIZE_MAX, Title, AppCommon.MSG_PROMPT_INPUT_NEW_PIN, this) == false) {
                return false;
            }
            if (PasswordBoxUtil.CheckEntrySize(passwordBoxPinConfirm, PIN_CODE_SIZE_MIN, PIN_CODE_SIZE_MAX, Title, AppCommon.MSG_PROMPT_INPUT_NEW_PIN_CONFIRM, this) == false) {
                return false;
            }

            // 数字入力チェック
            if (PasswordBoxUtil.CheckIsNumeric(passwordBoxPin, Title, AppCommon.MSG_PROMPT_INPUT_NEW_PIN_NUM, this) == false) {
                return false;
            }
            if (PasswordBoxUtil.CheckIsNumeric(passwordBoxPinConfirm, Title, AppCommon.MSG_PROMPT_INPUT_NEW_PIN_CONF_NUM, this) == false) {
                return false;
            }

            // 確認用PINコードのチェック
            if (PasswordBoxUtil.CompareEntry(passwordBoxPinConfirm, passwordBoxPin, Title, AppCommon.MSG_PROMPT_INPUT_PIN_CONFIRM_CRCT, this) == false) {
                return false;
            }

            return true;
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
