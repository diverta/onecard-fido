using System.Windows;
using ToolAppCommon;
using static MaintenanceToolApp.AppDefine;

namespace MaintenanceToolApp
{
    /// <summary>
    /// UtilityWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class UtilityWindow : Window
    {
        // 処理実行のためのプロパティー
        private readonly UtilityParameter Parameter;

        public UtilityWindow(UtilityParameter param)
        {
            // パラメーターの参照を保持
            Parameter = param;
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

        private void DoFlashROMInfo()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // 実行するユーティリティー機能の名称を設定し、画面を閉じる
            Parameter.Command = Command.COMMAND_HID_GET_FLASH_STAT;
            TerminateWindow(true);
        }

        private void DoFWVersionInfo()
        {
            // USB HID接続がない場合はエラーメッセージを表示
            if (WindowUtil.CheckUSBDeviceDisconnected(this)) {
                return;
            }

            // 実行するユーティリティー機能の名称を設定し、画面を閉じる
            Parameter.Command = Command.COMMAND_HID_GET_VERSION_INFO;
            TerminateWindow(true);
        }

        private void DoToolVersionInfo()
        {
            // 実行するユーティリティー機能の名称を設定し、画面を閉じる
            Parameter.Command = Command.COMMAND_VIEW_APP_VERSION;
            TerminateWindow(true);
        }

        private void DoViewLogFile()
        {
            // 実行するユーティリティー機能の名称を設定し、画面を閉じる
            Parameter.Command = Command.COMMAND_VIEW_LOG_FILE;
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
        private void buttonFlashROMInfo_Click(object sender, RoutedEventArgs e)
        {
            DoFlashROMInfo();
        }

        private void buttonFWVersionInfo_Click(object sender, RoutedEventArgs e)
        {
            DoFWVersionInfo();
        }

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
